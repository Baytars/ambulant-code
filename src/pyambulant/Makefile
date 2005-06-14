all: ambulantmodule.so

clean:
	rm ambulantobjgen.py ambulantincludegen.py ambulantmodule.cpp
	rm -r build
	
ambulantobjgen.py ambulantincludegen.py: genobjects.py 
	python genobjects.py
	
ambulantmodule.cpp: ambulantscan.py ambulantsupport.py ambulantgen.py ambulantobjgen.py ambulantincludegen.py
	python ambulantscan.py

ambulantmodule.so: ambulantmodule.cpp setup.py
	python setup.py build
