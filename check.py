#!/usr/bin/python

import re
import subprocess
import os.path
import sys

# MatSum  
program_matsum = ["./build/examples_cpu/matrixAdd/matrixAddCntrlCPU"]
input_matsum = ["2500","100"]
output_matsum = [580000.000000 ]
result_error = 0.000001

# MatMul
program_matmul=["./build/examples_cpu/matrixMult/matrixMultCntrlCPU"]
input_matmul = ["500","500"]
output_matmul = [5228325.000003 ]
result_error = 0.000001

# BlackScholes
program_black=["./build/examples_cpu/BlackScholes/BlackScholesCntrlCPU"]
input_black = ["2000","100"]
output_black = [1795.978613 ]
result_error = 0.000001

# CellAutom 
program_cell=["./build/examples_cpu/cellAutom/cellAutomCntrlCPU"]
input_cell = ["500","100"]
output_cell = [205.443172 ]
result_error = 0.000001

# MatSum GPU 
program_matsum_GPU = ["./build/examples_gpu/matrixAdd/matrixAddCntrlGPU"]
input_matsum_GPU = ["2500","100"]
output_matsum_GPU = [579999.094995]
result_error = 0.000001

# MatMul GPU 
program_matMult_GPU = ["./build/examples_gpu/matrixMult/matrixMultCntrlGPU"]
input_matMult_GPU = ["256","256"]
output_matMult_GPU = [1370571.641884]
result_error = 0.000001

# cellAutom GPU 
program_cellAutom_GPU = ["./build/examples_gpu/cellAutom/cellAutomCntrlGPU"]
input_cellAutom_GPU = ["1000","100"]
output_cellAutom_GPU = [338.645297]
result_error = 0.000001

# BlackScholes GPU 
program_BlackScholes_GPU = ["./build/examples_gpu/BlackScholes/BlackScholesCntrlGPU"]
input_BlackScholes_GPU = ["2500","100"]
output_BlackScholes_GPU = [200.856431]
result_error = 0.000001


#############################  GPU ##################################
def check_program_GPU(program,parallel,gpu,input_data,output_data,cola, machine):


	if not os.path.exists(program):
		print "Error: Program not found"
		sys.exit(0)
	
	print program + "\t[Size " + str(input_data)  +  "] \n",

	# Create the execution line
	exelist = []
	if parallel:
		exelist = ["srun"]
		exelist.append("--gres=gpu:1")
		exelist.append("-p")
		exelist.append(cola)
		exelist.append("-w")
		exelist.append(machine)
		exelist.append("-n")
		exelist.append("1")
		exelist.append(program)
		exelist.append(input_data[0])
		exelist.append(input_data[1])
		exelist.append(str(gpu))

	# Execute the program
	success = False
	subp = subprocess.Popen(exelist, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	for line in subp.stdout.readlines():
		result = re.search('Result: ([0-9\.]+)',line)
		if result:
			norm = float(result.group(1))
			error = abs(norm-output_data)
			if error < result_error:
				success = True
			
	retval = subp.wait()


	# Print the message
	print "[GPU " + str(gpu).rjust(2) +  "] ",
	if success:
		print "Ok"
	else:
		print "WRONG !!!" 

print 
#####################################################################



#############################  GPU ##################################
def check_program(program,parallel,threads,input_data,output_data,cola, machine):


	if not os.path.exists(program):
		print "Error: Program not found"
		sys.exit(0)
	
	print program + "\t[Size " + str(input_data)  +  "] \n",

	# Loop for the processes
	for p in threads:
	
		# Create the execution line
		exelist = []
		if parallel:
			exelist = ["srun"]
			exelist.append("-p")
			exelist.append(cola)
			exelist.append("-w")
			exelist.append(machine)
			exelist.append("-n")
			exelist.append("1")
			exelist.append(program)
			exelist.append(input_data[0])
			exelist.append(input_data[1])
			exelist.append(str(p))

		# Execute the program
		success = False
		subp = subprocess.Popen(exelist, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		for line in subp.stdout.readlines():
			result = re.search('Result: ([0-9\.]+)',line)
			if result:
				norm = float(result.group(1))
				error = abs(norm-output_data)
				if error < result_error:
					success = True
			
		retval = subp.wait()


		# Print the message
		print "[Procs " + str(p).rjust(2) +  "] ",
		if success:
			print "Ok"
		else:
			print "WRONG !!!" 

	print 

#####################################################################


## Cross programs
for j in range(1):
	# Message MatSum
	print "Checking:" +  program_matsum[0]
	print "------------------------"
	check_program(program_matsum[0],True,[1,2,4,16,22,33],input_matsum,output_matsum[0], "dark-night", "frontend")
	
	# Message MatSum
	print "Checking:" +  program_matmul[0]
	print "------------------------"
	check_program(program_matmul[0],True,[1,2,4,16,22,33],input_matmul,output_matmul[0], "dark-night", "frontend")
	
	# Message Black
	print "Checking:" +  program_black[0]
	print "------------------------"
	check_program(program_black[0],True,[1,2,4,16,22,33],input_black,output_black[0], "dark-night", "frontend")
	
	# Message CellAutom
	print "Checking:" +  program_cell[0]
	print "------------------------"
	check_program(program_cell[0],True,[1,2,4,16,22,33],input_cell,output_cell[0], "dark-night", "frontend")


	# Message GPU MatSum
	print "Checking:" +  program_matsum_GPU[0]
	print "------------------------"
	check_program_GPU(program_matsum_GPU[0],True,0,input_matsum_GPU,output_matsum_GPU[0], "dark-night", "hydra")
	
	# Message GPU cellAutom 
	print "Checking:" +  program_cellAutom_GPU[0]
	print "------------------------"
	check_program_GPU(program_cellAutom_GPU[0],True,0,input_cellAutom_GPU,output_cellAutom_GPU[0], "dark-night", "hydra")

	# Message GPU MatMult
	print "Checking:" +  program_matMult_GPU[0]
	print "------------------------"
	check_program_GPU(program_matMult_GPU[0],True,0,input_matMult_GPU,output_matMult_GPU[0], "dark-night", "hydra")
	
	# Message GPU BlackScholes 
	print "Checking:" +  program_BlackScholes_GPU[0]
	print "------------------------"
	check_program_GPU(program_BlackScholes_GPU[0],True,0,input_BlackScholes_GPU,output_BlackScholes_GPU[0], "dark-night", "hydra")
	
