#include "OOPPluginManager.h"
#include "LogMacros.h"

using namespace Buteo;

OOPPluginManager::OOPPluginManager()
{
    FUNCTION_CALL_TRACE;
}

OOPPluginManager::~OOPPluginManager()
{
    FUNCTION_CALL_TRACE;
}

OOPClientPlugin* OOPPluginManager::createClient( const QString &aPluginName,
                                                 const SyncProfile &aProfile )
{
    FUNCTION_CALL_TRACE;
}

void OOPPluginManager::destroyClient( OOPClientPlugin *aPlugin )
{
    FUNCTION_CALL_TRACE;
}

OOPServerPlugin* OOPPluginManager::createServer( const QString &aPluginName,
                                                 const Profile &aProfile )
{
    FUNCTION_CALL_TRACE;
}

void OOPPluginManager::destroyServer( OOPServerPlugin *aPlugin )
{
    FUNCTION_CALL_TRACE;
}
