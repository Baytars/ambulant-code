all: ambulantmodule.so

ambulantobjgen.py: genobjects.py
	python genobjects.py
	
ambulantmodule.cpp: ambulantscan.py ambulantsupport.py ambulantgen.py ambulantobjgen.py
	python ambulantscan.py

ambulantmodule.so: ambulantmodule.cpp setup.py
	python setup.py build
