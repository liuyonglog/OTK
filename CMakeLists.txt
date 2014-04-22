cmake_minimum_required (VERSION 2.6)


project (OTK)



FIND_PACKAGE( Qt4 REQUIRED )

FIND_PACKAGE( OpenCV REQUIRED )


#find_package (OpenGL REQUIRED)


set(SAMPLE_SRCS
  main.cpp
  mainwindow.cpp
  imgWidget.cpp
)

set(SAMPLE_HDRS
  mainwindow.h
  imgWidget.h
)

set(SAMPLE_UIS
  mainwindow.ui
)

set(SAMPLE_MOC_HDRS ${SAMPLE_HDRS})

qt4_wrap_ui( SAMPLE_UI_HDRS ${SAMPLE_UIS})
qt4_wrap_cpp( SAMPLE_MOC_SRCS ${SAMPLE_MOC_HDRS})


include(${QT_USE_FILE})

include_directories(${CMAKE_BINARY_DIR})

ADD_EXECUTABLE(OTKApp ${SAMPLE_SRCS} ${SAMPLE_MOC_SRCS} ${SAMPLE_UI_HDRS})
TARGET_LINK_LIBRARIES(OTKApp ${OpenCV_LIBS} ${QT_LIBRARIES})

#TARGET_LINK_LIBRARIES(OpenGL_Qt_Binding ${OpenCV_LIBS} ${OPENGL_LIBRARIES} )