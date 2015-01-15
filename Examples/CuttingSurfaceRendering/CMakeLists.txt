PROJECT (VtkExamples)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#VTK
FIND_PACKAGE(VTK)
 IF(VTK_FOUND)
   INCLUDE (${USE_VTK_FILE})
   LINK_LIBRARIES( vtkHybrid vtkRendering  vtkVolumeRendering  vtkGraphics vtkImaging vtkIO vtkFiltering vtkCommon )
   IF (${VTK_MAJOR_VERSION} GREATER 4)
      LINK_LIBRARIES( vtkWidgets )
   ENDIF (${VTK_MAJOR_VERSION} GREATER 4)
 ENDIF(VTK_FOUND)

#SRC
SET (LV_SRCS CuttingSurfaceRendering.cpp)
 

ADD_EXECUTABLE (VtkExamples ${LV_SRCS})