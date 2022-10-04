cmake_minimum_required(VERSION 2.6)
project(jens_repair)

#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(SOURCE_FILES Main 
		 Services
		 SurfTriangle
		 SurfEdgeContainer
		 SurfPair
		 SurfEdge
		 Point3D 
		 Hexahedron 
		 Prism 
		 Pyramid 
		 Tetrahedron
		 TriMesh
		 FaceContainer
		 Face
		 Element
		 Octant
		 OctreeEdge
		 Quality
		 Metric
		 MetricJens
		 MetricJR
		 MeshPoint
		 math/Matrix 
		 math/Polynom 
		 math/PolyMatrix 
		 math/Vector
		 Visitors/Visitor
        )

add_executable(jens_repair ${SOURCE_FILES})
