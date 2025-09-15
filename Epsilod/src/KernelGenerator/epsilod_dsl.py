import argparse
from collections import defaultdict
from dataclasses import dataclass
import dataclasses
import itertools
import os
from typing import Any, Iterator


@dataclass
class Stencil:
	stencil: str
	type: str
	shape: str
	weight_divisor: str = ""
	kernel: str = ""
	defs: str = ""
	ext_params: str = ""
	weights_geometry: str = ""
	weights_list: str = ""

	@property
	def dims(self) -> int:
		return len(self.shp_ranges)

	@property
	def shp_ranges(self) -> list[range]:
		ranges = self.shape.removeprefix("[").removesuffix("]").split("][")
		return [range(int(start), int(end) + 1) for start, end in map(lambda r: r.split(","), ranges)]

	def get_weight_div(self) -> str:
		if self.weight_divisor != "":
			return self.weight_divisor
		return str(sum(map(float, self.weights_dict().values())))

	def weights_dict(self) -> defaultdict[tuple[int, ...], Any]:
		if self.weights_geometry != "":
			d = dict(zip(self.shape_iter(), map(lambda w: w.strip(), self.weights_geometry.split(","))))
			return defaultdict(float, {k: v for k, v in d.items() if float(v) != 0.0})
		if self.weights_list != "":
			return defaultdict(float, eval(f"{{{self.weights_list}}}"))
		raise ValueError(f"Error no weights data found for stencil {self.stencil}")

	def shape_iter(self) -> Iterator[tuple[int, ...]]:
		return itertools.product(*self.shp_ranges)

	def gen_kernel(self):
		ijk = [chr(ord('i') + i) for i in range(self.dims)]
		access_ijk = ", ".join(f"thr_{c}" for c in ijk)

		if self.kernel != "":
			# add implicit stuff to custom kernel and undo amogus replace
			self.kernel = f"{self.type} new = old;\n" + self.kernel.replace("ඞ", ":")
			self.kernel += f"hit( matrix, {access_ijk} ) = new;\n"
			return

		self.kernel += f"hit(matrix, {access_ijk}) = (\n"
		for coord, weight in self.weights_dict().items():
			offset_ijk = ", ".join([f"thr_{idx} + {offset}" for idx, offset in zip(ijk, coord)])
			self.kernel += f"hit(matrixCopy, {offset_ijk}) * {weight} + \n"
		self.kernel = self.kernel.removesuffix(" + \n")
		self.kernel += f") / {self.get_weight_div()};"

	def gen_kernel_file(self):
		# generate kernel code if custom kernel was not specified
		self.gen_kernel()

		with open(f"{self.stencil}.c", "w") as f:
			if self.ext_params != "":
				f.write(f'#include "{self.stencil}_ext_type.h"\n')
			f.write('#include "epsilod_kernels.h"\n')

			# TODO what to do for 4d coords? thr_l is not generated

			ijk = [chr(ord('i') + i) for i in range(self.dims)]
			arg_ijk = ", ".join([f"_{c}" for c in ijk])
			offset_ijk = ", ".join([f"_{c} + thr_{c}" for c in ijk])
			zeroes = ", ".join("0" * self.dims)

			for d in self.defs.splitlines():
				f.write(f"#define {d}\n")

			f.write(f"#define neigh({arg_ijk}) hit(matrixCopy, {offset_ijk})\n")
			f.write(f"#define old neigh({zeroes})\n")

			f.write(
			    f"CTRL_KERNEL({self.stencil}, GENERIC, DEFAULT, KHitTile_{self.type} matrix, KHitTile_{self.type} matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {{\n"
			)
			f.write(f"{self.kernel}\n")
			f.write("});\n")
			f.write("#undef neigh\n")
			f.write("#undef old\n")
			# TODO undef defs?
		# symlinks for CUDA and HIP kernel files
		os.symlink(f"{self.stencil}.c", f"{self.stencil}.cu")
		os.symlink(f"{self.stencil}.c", f"{self.stencil}.cpp")

	def gen_data_header(self):
		with open(f"{self.stencil}_data.h", "w") as f:
			shp = self.shape.replace("[", "(").replace("]", ")").replace(")(", "),(")

			f.write(f"REGISTER_STENCIL({self.stencil}, GENERIC, DEFAULT);\n")
			f.write(f"HitShape shp_{self.stencil} = hitShape({shp});\n")

			geometry = ", ".join([str(self.weights_dict()[c]) for c in self.shape_iter()])
			f.write(f"float stencilData_{self.stencil}[] = {{{geometry}}};\n")
			f.write(f"float factor_{self.stencil} = {self.get_weight_div()};\n")

	def gen_exttype_header(self):
		if self.ext_params == "":
			return
		with open(f"{self.stencil}_ext_type.h", "w") as f:
			f.write("#define EPSILOD_USER_TYPES typedef struct {")
			for param in self.ext_params.splitlines():
				f.write(f"{param};")
			f.write("} Epsilod_ext;")


def clean_str(stencil: str) -> str:
	lines = [l.strip() for l in stencil.splitlines()]
	lines = [l for l in lines if l != "" and not l.startswith("#")]
	return "\n".join(lines)


def sanity_check(stencil: str):
	for kw in dataclasses.fields(Stencil):
		if stencil.count(f"kw:"):
			raise ValueError(f"Error field {kw} appears more than once")
	if "weights_geometry:" in stencil and "weights_list:" in stencil:
		raise ValueError("Error mutually exclusive operations weights_geometry and weights_list specified")


if __name__ == "__main__":
	parser = argparse.ArgumentParser(description = "Generate kernel files for EPSILOD.")

	parser.add_argument("path", help = "Path to the kernel descriptor file")
	args = parser.parse_args()

	with open(args.path, "r") as f:
		# temporarely replace : for another character in comments to avoid breaking stuff
		stencils = "\n".join([l.replace(":", "ඞ") if l.strip().startswith("#") else l for l in f.read().split("\n")]).split("stencil:")[1:]

	for st in stencils:
		sanity_check(st)
		for kw in dataclasses.fields(Stencil):
			st = st.replace(f"{kw.name}:", f"''', '{kw.name}': '''")
		d = eval(f"{{'stencil':'''{st}'''}}")
		for k, v in d.items():
			if k != "kernel":
				d[k] = clean_str(v)
		stencil = Stencil(**d)
		stencil.gen_kernel_file()
		stencil.gen_exttype_header()
		stencil.gen_data_header()
