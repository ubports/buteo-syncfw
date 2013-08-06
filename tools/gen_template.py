#!/usr/bin/python
'''
*
* Copyright (C) 2013 Jolla Ltd. and/or its subsidiary(-ies).
*
* Author: Sateesh Kavuri <sateesh.kavuri@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
'''

from Cheetah.Template import Template
from configobj import ConfigObj

import sys, argparse, os, shutil
import errno

def main():
    usage = "usage: %prog [options] arg"

    parser = argparse.ArgumentParser (description='Generates Buteo sync plugin template code', usage=sys.argv[0] + ' [options] arg')
    parser.add_argument('-c', '--config', help="Config file to generate the source templates", required=True)
    parser.add_argument('-d', '--outdir', help="Target output directory. Should be writable")
    args = parser.parse_args()
    options = vars (args)
    if options['config'] == None or options['outdir'] == None:
        print parser.print_help()
        sys.exit(2)

    # Read the configuration file provided from cmdline
    co = ConfigObj (options['config'])

    # Validate the config file
    validator = ConfigValidator (co)
    validator.validate ()

    tg = TemplateGenerator (co, options['outdir'])
    tg.createDirStructure()
    tg.generateClasses()
    tg.generateProfiles()
    tg.generateProjectFile()

'''
    Class to validate the configuration input
'''
class ConfigValidator:
    def __init__(self, configObj):
        self.mandatory_client_config_keys  = set(['classname', 'name', 'destinationtype', 'schedule', 'displayname', 'transport', 'enabled', 'agent', 'syncdirection', 'conflictpolicy'])
        self.mandatory_server_config_keys  = set(['classname', 'name', 'transport', 'enabled'])
        self.mandatory_storage_config_keys = set(['classname', 'name', 'mimetype', 'enabled'])
        self.mandatory_fields = set ([])
        self.configObj = configObj

        if configObj.has_key ('config') == False:
            print "Error: Invalid config file. 'config' section not available"
            sys.exit (3)

        if configObj.get('config').has_key('type') == False:
            print "Error: Invalid config file. 'type' of the plugin not provided. Possible values: 'client/server/storage'"
            sys.exit (3)

        if configObj['config']['type'] == 'client':
            self.mandatory_fields = self.mandatory_client_config_keys
        elif configObj['config']['type'] == 'server':
            self.mandatory_fields = self.mandatory_server_config_keys
        elif configObj['config']['type'] == 'storage':
            self.mandatory_fields = self.mandatory_storage_config_keys
        else:
            suppliedtype = configObj['config']['type']
            print "Error: Invalid plug-in type '" + suppliedtype + "' specified. Should be one of 'clientconfig/serverconfig/storageconfig'"
            sys.exit (3)

    def validate (self):
        for key in self.mandatory_fields:
            if (self.configObj.has_key(key) and (self.configObj.get(key).strip () == '') and (self.configObj.get(key) == None)):
                print "Error: Value for '" + key + "' cannot be empty"
                sys.exit (3)

        fields = set (self.configObj.get('config').keys())
        missing_fields = self.mandatory_fields - fields
        if len(missing_fields) != 0:
           print "Missing mandatory fields:" + str(missing_fields)
           sys.exit (3)

        # Validate specific server profile properties
        if self.configObj['config']['type'] == 'server':
            self.validateServerProfile(self.configObj['config'])

    def validateServerProfile (self, properties):
        # For now, possible 'transport' key/values are checked
        possible_transports = ['usb', 'bt', 'ip']
        given_transport = properties['transport']
        if possible_transports.__contains__(given_transport) == None:
                print "Wrong value for transport. Possible values: usb/bt/ip"
                sys.exit(3)

# End validator class

'''
Class to generate the template
'''
class TemplateGenerator:
    def __init__ (self, configObj, targetDir):
        self.targetDir = targetDir
        self.configObj = configObj
        TEMPLATE_DIR = 'templates/'

        if self.configObj['config']['type'] == 'client':
            self.classtemplate_h = TEMPLATE_DIR + 'clientplugin_h.tmpl'
            self.classtemplate_cpp = TEMPLATE_DIR + 'clientplugin_cpp.tmpl'
            self.profile = TEMPLATE_DIR + 'syncprofile_xml.tmpl'
            self.agent_profile = TEMPLATE_DIR + 'syncagentprofile_xml.tmpl'
        elif self.configObj['config']['type'] == 'server':
            self.classtemplate_h = TEMPLATE_DIR + 'serverplugin_h.tmpl'
            self.classtemplate_cpp = TEMPLATE_DIR + 'serverplugin_cpp.tmpl'
            self.profile = TEMPLATE_DIR + 'serverprofile_xml.tmpl'
        elif self.configObj['config']['type'] == 'storage':
            self.classtemplate_h = TEMPLATE_DIR + 'storageplugin_h.tmpl'
            self.classtemplate_cpp = TEMPLATE_DIR + 'storageplugin_cpp.tmpl'
            self.profile = TEMPLATE_DIR + 'storageprofile_xml.tmpl'

        self.projectfile = TEMPLATE_DIR + 'myproj_pro.tmpl'

    def hasStorages (self):
        if self.configObj.has_key ('storages') and (len(self.configObj['storages']) > 0):
            return True
        else:
            return False

    def hasExtConfiguration (self):
        if self.configObj.has_key ('ext-config') and (len (self.configObj['ext-config']) > 0):
            return True
        else:
            return False

    def createDirStructure (self):
        dirExistsFlag = None
        dir = self.targetDir
        while dirExistsFlag != False:
            if os.path.exists (dir):
                print "Directory '" + dir + "' already exists"
                dir = raw_input("Enter new directory (Press enter for exiting):")
                if dir.strip() == None:
                    sys.exit(4)
                if os.path.exists(dir):
                    dirExistsFlag = True
                    break
                else:
                    dirExistsFlag = False
                    continue
            else:
                dirExistsFlag = False

        self.targetDir = dir
        try:
            os.makedirs(dir + "/" + "xml")

            if self.configObj['config']['type'] == 'client':
                os.makedirs(dir + "/xml/client")
                # Create the sync directory
                os.makedirs(dir + "/xml/sync")
            elif self.configObj['config']['type'] == 'server':
                os.makedirs(dir + "/xml/server")
            if self.configObj['config']['type'] == 'storage':
                os.makedirs(dir + "/xml/storage")

        except OSError as exception:
            if exception.errno != errno.EEXIST:
                raise

    def cleanup (self):
        inp = 'yn'
        count = 1
        while inp.lower() == 'y' or inp.lower() == 'n' or count < 4:
            inp = raw_input("Really delete '" + self.targetDir + "' (y/n)?")
            if inp.lower() == 'y':
                shutil.rmtree(self.targetDir, False)
                return
            elif inp.lower() == 'n':
                return
            count = count + 1

    def generateClasses (self):
        classname = self.configObj['config']['classname']

        # Generate .h
        header = open (self.targetDir + "/" + classname + ".h", "w")
        print >> header, Template (file = self.classtemplate_h, searchList = [{'plugin':self.configObj.get('config')}])
        header.close()

        # Generate .cpp
        cpp = open (self.targetDir + "/" + classname + ".cpp", "w")
        print >> cpp, Template (file = self.classtemplate_cpp, searchList = [{'plugin':self.configObj.get('config')}])
        cpp.close()

    def generateStorageProfiles (self, profileXmlName):
        sl = dict()
        sl['profile'] = self.configObj['config']
        if self.configObj.get('config').has_key('ext-config'):
            sl['extprops'] = self.configObj['config']['ext-config']

        storage_file = open (self.targetDir + "/xml/storage/" + profileXmlName + ".xml", "w")
        print >> storage_file, Template (file = self.profile, searchList = [sl])
        storage_file.close()

    def generateSyncAgentProfile (self, profileXmlName):
        sl = dict()
        sl['name'] = profileXmlName
        sl['props'] = self.configObj['config']['agent']['props']

        agent_file = open (self.targetDir + "/xml/client/" + profileXmlName + ".xml", "w")
        print >> agent_file, Template (file = self.agent_profile, searchList = [sl])
        agent_file.close()

    def generateServerProfile (self, profileXmlName):
        sl = dict()
        sl['profile'] = self.configObj['config']
        if self.configObj.get('config').has_key('storages'):
            sl['storages'] = self.configObj['config']['storages']
        if self.configObj.get('config').has_key('ext-config'):
            sl['extprops'] = self.configObj['config']['ext-config']

        server_profile = open (self.targetDir + "/xml/server/" + profileXmlName + ".xml", "w")
        print >> server_profile, Template (file = self.profile, searchList = [sl])
        server_profile.close()

    def generateSyncProfile (self, profileXmlName):
        sl = dict()
        sl['profile'] = self.configObj['config']
        if self.configObj.get('config').has_key('storages'):
            sl['storages'] = self.configObj['config']['storages']
        sl['agentname'] = self.configObj['config']['agent']['name']
        sl['agentprops'] = self.configObj['config']['agent']['props']
        if self.configObj.get('config').has_key('ext-config'):
            sl['extprops'] = self.configObj['config']['ext-config']

        sync_profile_file = open (self.targetDir + "/xml/sync/" + profileXmlName + ".xml", "w")
        print >> sync_profile_file, Template (file = self.profile, searchList = [sl])
        sync_profile_file.close()

    def generateProfiles (self):
        profile_name = self.configObj['config']['name']

        if self.configObj['config']['type'] == 'client':
            agent_name = self.configObj['config']['agent']['name']
            self.generateSyncProfile(profile_name)
            self.generateSyncAgentProfile(agent_name)
        elif self.configObj['config']['type'] == 'server':
            self.generateServerProfile(profile_name)
        elif self.configObj['config']['type'] == 'storage':
            self.generateStorageProfiles(profile_name)

        self.generateProjectFile()

    def generateProjectFile (self):
        sl = dict()
        sl['projectname'] = self.targetDir + "-" + self.configObj['config']['type']
        sl['profile'] = {'type':self.configObj['config']['type'], }
        sl['classname'] = self.configObj['config']['classname']

        project_file = open (self.targetDir + "/" + self.targetDir + ".pro", "w")
        print >> project_file, Template (file = self.projectfile, searchList = [sl])
        project_file.close()

# End TemplateGenerator class

# Main function
if __name__ == "__main__":
    main()

