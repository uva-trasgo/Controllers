from dataclasses import dataclass, field
from typing import Any, Protocol


class DeviceConfig(Protocol):

	def get_ref_params(self) -> str:
		...

	@property
	def arch(self) -> str:
		...


@dataclass
class CudaDevice:
	dev: int
	kstreams: int = 1
	align: int = 0

	def get_ref_params(self) -> str:
		return f"{self.dev}"

	@property
	def arch(self) -> str:
		return "cuda"


@dataclass
class HipDevice:
	dev: int
	kstreams: int = 1
	align: int = 0

	def get_ref_params(self) -> str:
		return f"{self.dev}"

	@property
	def arch(self) -> str:
		return "hip"


@dataclass
class OpenCLDevice:
	platform: int
	dev: int
	kstreams: int = 1
	align: int = 0

	def get_ref_params(self) -> str:
		return f"{self.dev} {self.platform}"

	@property
	def arch(self) -> str:
		return "opencl"


@dataclass
class CPUDevice:
	threads: int
	numa: tuple[int, ...] = (0, 0)
	transfers: int = 0
	align: int = 0

	def get_ref_params(self) -> str:
		return f"{self.threads}"

	@property
	def arch(self) -> str:
		return "cpu"


@dataclass
class FPGADevice:
	platform: int
	dev: int
	exec_mode: int
	kstreams: int = 1
	align: int = 0

	def get_ref_params(self) -> str:
		return f"{self.dev} {self.platform} {self.exec_mode}"

	@property
	def arch(self) -> str:
		return "fpga"


@dataclass
class ProcConfig:
	node: str
	rank: int
	weight: float = 1
	numa: int = 0
	devices: list[DeviceConfig] = field(default_factory = list)


def kvl2dict(kvl: list[str]) -> dict[str, Any]:
	kvl = [s for s in kvl if s not in ("", "=")]
	return {k: Config.KV_ARG_TYPES[k](v) for k, v in zip(kvl[::2], kvl[1::2])}


class Config:
	procs: list[ProcConfig]
	path: str
	KV_ARG_TYPES = {
	    "threads": int,
	    "numa_range": lambda x: tuple(map(int, x.split("-"))),
	    "memmoves": int,
	    "platform": int,
	    "dev": int,
	    "kstreams": int,
	    "align": int,
	    "numa": int,
	    "weight": float,
	}
	DEV_STR2CLS = {
	    "cuda": CudaDevice,
	    "hip": HipDevice,
	    "opencl": OpenCLDevice,
	    "fpga": FPGADevice,
	    "cpu": CPUDevice,
	}

	def __init__(self, path: str):

		self.path = path
		self.procs = []

		with open(path, "r") as f:
			data = [l.strip() for l in f.read().split("\n") if l.strip() != ""]
			curr_node_name = ""
			for l in data:
				match l.replace("=", " = ").split(" "):
					case ["node", name]:
						curr_node_name = name
					case ["proc", rank, *args]:
						self.procs.append(ProcConfig(curr_node_name, int(rank), **kvl2dict(args)))
					case [arch, *args]:
						self.procs[-1].devices.append(Config.DEV_STR2CLS[arch](**kvl2dict(args)))
					case _:
						raise ValueError(f"Error: bad config format on {path}")

			if len(self.procs) != len({p.rank for p in self.procs}):
				raise ValueError(f"Error: repeated process rank in config {path}")

	def get_march_flags(self, cc: str) -> str:
		"""Return the apropiate optimization flags based on the node and compiler used."""
		# TODO flags for medusa?
		# TODO clang compiler flags?
		match list({proc.node for proc in self.procs}):
			case ["manticore"]:
				if cc in ("gcc", "icc"):
					return "march=skylake-avx512"
			case ["gorgon"]:
				if cc in ("gcc", "icc"):
					return "march=znver3"
				elif cc == "icc":
					return "march=core-avx2"
		return ""
