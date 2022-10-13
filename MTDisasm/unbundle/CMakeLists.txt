cmake_minimum_required(VERSION 3.2)
project(unbundle)

set(SOURCE_FILES
	Catalog.cpp
	CFileIOStream.cpp
	DataObject.cpp
	DataReader.cpp
	Endian.cpp
	MemIOStream.cpp
	MTDisasm.cpp
	SliceIOStream.cpp
	stb_image_write.c
	)

add_executable(unbundle ${SOURCE_FILES})

set_property(TARGET unbundle PROPERTY CXX_STANDARD 11)
set_property(TARGET unbundle PROPERTY CXX_STANDARD_REQUIRED ON)
