include_directories(${LIB_UTIL})

list(APPEND UtilSourceFiles
        buffer.cc)

foreach(file ${UtilSourceFiles})
    list(APPEND ProjectSourceFiles ${LIB_UTIL}/${file})
endforeach()