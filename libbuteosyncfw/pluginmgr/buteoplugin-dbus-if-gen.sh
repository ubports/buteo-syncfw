# For client interface
qdbusxml2cpp -v -c ButeoPluginIface -p ButeoPluginIface.h:ButeoPluginIface.cpp com.buteo.msyncd.baseplugin.xml

# For server interface
qdbusxml2cpp -c ButeoPluginIfaceAdaptor -a ButeoPluginIfaceAdaptor.h:ButeoPluginIfaceAdaptor.cpp com.buteo.msyncd.baseplugin.xml
