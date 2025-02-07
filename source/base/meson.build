# DOSBox Core sources

DOSBoxSrc =  [
]

DOSBoxIncludeDirs = [
  '.',
  '..',
]

DOSBoxCompileArgs = [
	'-Wfatal-errors',

]

# DOSBox dependency

 DOSBoxDependency = declare_dependency(
  compile_args        : [  DOSBoxCompileArgs  ],
		link_args        : [  ],
  include_directories : include_directories(DOSBoxIncludeDirs),
  sources             : [ DOSBoxSrc ],
  dependencies        : [  
                        ]
 )

# DOSBox Core Configuration

 baseDependency = declare_dependency(
  compile_args        : [ ],
  include_directories : [ include_directories( ['.', '..'] )],
  sources             : [ ],
  dependencies        : [ 
			                       DOSBoxDependency,
                        ]
 )
