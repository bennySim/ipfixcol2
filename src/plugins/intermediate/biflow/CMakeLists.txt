# Create a linkable module
add_library(biflow-intermediate MODULE
        biflow.cpp Config.hpp Storage.cpp Storage.hpp MsgGen.cpp MsgGen.h Builder.cpp Builder.hpp Generator.cpp Generator.h Converter.hpp IanaDefinitions.h IPAddr.cpp IPAddr.hpp Converter.cpp DataStructures.h DataStructures.cpp BiflowTemplate.cpp BiflowTemplate.h TimeExpiration.cpp TimeExpiration.h MsgSender.cpp MsgSender.h TemplatesHandler.cpp TemplatesHandler.h)

install(
        TARGETS biflow-intermediate
        LIBRARY DESTINATION "${INSTALL_DIR_LIB}/ipfixcol2/"
)