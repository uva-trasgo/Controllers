stencil: cc2d4
	type: float
	shape: [-1,1][-1,1]
	weights_list:  
		(-1,0):1,
		(0,-1):1,
		(0,1):1,
		(1,0):1,

stencil: cc2d8
	type: float
	shape: [-1,1][-1,1]
	weights_list:  
		1, 4, 1,
		4, 0, 4,
		1, 4, 1

stencil: cc2dnc9
	type: float
	shape: [-2, 2][-2, 2]
	weights_list:  
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0,
		1, 1, 1, 1, 1,
		0, 0, 1, 0, 0,
		0, 0, 1, 0, 0
		
stencil: cc2df5
	type: float
	shape: [-2, 0][-2, 0]
	weights_list:  
		0, 0, 1,
		0, 0.5, 2,
		1, 2, 0