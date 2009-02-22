objs = read_dxf.o entities.o entities2elements.o tables.o tables2svg_info.o blocks.o
flags = -O1

dxf2svg : dxf2svg.cpp $(objs)
	g++ $(flags) -o dxf2svg dxf2svg.cpp $(objs)

test : test_dxf.cpp $(objs)
	g++ $(flags) -o test test_dxf.cpp $(objs)
	
%.o : %.cpp %.h 
	g++ $(flags) -c $<
	
clean :
	rm dxf2svg $(objs)

install:
	echo ****User intervention required****
	echo
	echo Copy dxf2svg executable and dxf2svg.inx to share/extension directory
	echo Make sure file permissions are set correctly
	
