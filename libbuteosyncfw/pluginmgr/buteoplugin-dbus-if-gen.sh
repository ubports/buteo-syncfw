# For client interface
qdbusxml2cpp -v -c ButeoPluginIf -p ButeoPluginIf.h:ButeoPluginIf.cpp com.buteo.msyncd.baseplugin.xml

# For server interface
qdbusxml2cpp -c ButeoPluginIfAdaptor -a ButeoPluginIfAdaptor.h:ButeoPluginIfAdaptor.cpp com.buteo.msyncd.baseplugin.xml
