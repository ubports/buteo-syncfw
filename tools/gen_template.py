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

import sys, ConfigParser, argparse, os, shutil

def main():
    usage = "usage: %prog [options] arg"

    parser = argparse.ArgumentParser (description='Generates Buteo sync plugin template code', usage=sys.argv[0] + ' [options] arg')
    parser.add_argument('-c', '--config')
    args = parser.parse_args()
    if not args.config:
        print parser.usage
        sys.exit(2)

    options = vars (args)

    # Read the configuration file provided from cmdline
    cf = ConfigParser.ConfigParser ()
    cf.read (options['config'])

    # Validate the config file
    validator = ConfigValidator (cf)
    validator.validate ()

    tg = TemplateGenerator (cf, "test")
    tg.createDirStructure()
    tg.generateClasses()
    tg.generateProfiles()

'''
    Class to validate the configuration input
'''
class ConfigValidator:
    def __init__(self, configParser):
        self.mandatory_client_config_keys  = set(['classname', 'name', 'destinationtype', 'schedule', 'displayname', 'transport', 'enabled', 'agent_name', 'syncdirection', 'conflictpolicy'])
        self.mandatory_server_config_keys  = set(['classname', 'name', 'transport', 'enabled'])
        self.mandatory_storage_config_keys = set(['classname', 'name', 'mimetype', 'enabled'])
        self.type = ''
        self.suppliedtype = ''
        self.mandatory_fields = set ([])
        self.values = {}

        if configParser.sections ().__contains__('clientconfig'):
            self.type = 'clientconfig'
            self.mandatory_fields = self.mandatory_client_config_keys
        elif configParser.sections ().__contains__('serverconfig'):
            self.type = 'serverconfig'
            self.mandatory_fields = self.mandatory_server_config_keys
        elif configParser.sections ().__contains__('storageconfig'):
            self.type = 'storageconfig'
            self.mandatory_fields = self.mandatory_storage_config_keys
        else:
            self.suppliedtype = configParser.sections ()[0]
            print "Error: Invalid plug-in type '" + self.suppliedtype + "' specified. Should be one of 'clientconfig/serverconfig/storageconfig'"
            sys.exit (3)

        self.values = dict (configParser.items (self.type))

    def validate (self):
        fields = set([])
        for key, val in self.values.iteritems ():
            if (self.mandatory_fields.__contains__(key) and (val.strip () == '') and (len(val.strip ()) == 0)):
                print "Error: Value for '" + key + "' cannot be empty"
                sys.exit (3)
            else:
                fields.add(key)

        missing_fields = self.mandatory_fields - fields
        if (missing_fields):
           print "Missing mandatory fields:"
           for key in missing_fields:
               print key
           sys.exit (3)

# End validator class

'''
Class to generate the template
'''
class TemplateGenerator:
    def __init__ (self, configParser, targetDir):
        self.targetDir = targetDir
        self.configParser = configParser

        self.type = ''
        if self.configParser.has_section ('clientconfig'):
            self.type = 'clientconfig'
            self.classtemplate_h = 'clientplugin_h.tmpl'
            self.classtemplate_cpp = 'clientplugin_cpp.tmpl'
            self.profile = 'syncprofile_xml.tmpl'
        elif self.configParser.has_section ('serverconfig'):
            self.type = 'serverconfig'
            self.classtemplate_h = 'serverplugin_h.tmpl'
            self.classtemplate_cpp = 'serverplugin_cpp.tmpl'
            self.profile = 'serverprofile_xml.tmpl'
        elif self.configParser.has_section ('storageconfig'):
            self.type = 'storageconfig'
            self.classtemplate_h = 'storageplugin_h.tmpl'
            self.classtemplate_cpp = 'storageplugin_cpp.tmpl'
            self.profile = 'storageprofile_xml.tmpl'

    def createDirStructure (self):
        try:
            if not os.path.exists(self.targetDir):
                os.makedirs(self.targetDir + "/" + "xml")
        except OSError as exception:
            if exception.errorno != errno.EEXIST:
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
        classname = self.configParser.get (self.type, 'classname')

        # Generate .h
        header = open (self.targetDir + "/" + classname + ".h", "w")
        print >> header, Template (file = self.classtemplate_h, searchList = [{'plugin':dict(self.configParser.items(self.type))}])
        header.close()

        # Generate .cpp
        cpp = open (self.targetDir + "/" + classname + ".cpp", "w")
        print >> cpp, Template (file = self.classtemplate_cpp, searchList = [{'plugin':dict(self.configParser.items(self.type))}])
        cpp.close()
    
    def generateProfiles (self):
        searchList = dict ()
        searchList['profile'] = dict (self.configParser.items (self.type))
        searchList['storage'] = dict (self.configParser.items ('storages'))
        searchList['extconfig'] = dict (self.configParser.items ('ext-config'))

        # Create the dir's for the xml files
        if len (searchList) != 0:
            os.makedirs(self.targetDir + "/xml/sync")
            os.makedirs(self.targetDir + "/xml/client")
        if searchList.has_key('storage'):
            os.makedirs(self.targetDir + "/xml/storage")
        
        profile_name = self.configParser.get (self.type, 'name')
        agent_name = self.configParser.get (self.type, 'agent_name')
        #storage_name = self.configParser.get ('storages', 'TBD')
        sync_profile = open (self.targetDir + "/xml/sync/" + profile_name + ".xml", "w")
        #client_profile = open (self.targetDir + "/xml/client/" + agent_name + ".xml", "w")
        #storage_profile = open (self.targetDir + "/xml/storage/" + )

        print >> sync_profile, Template (file = self.profile, searchList = [searchList])
        #print >> client_profile, Template (file = )
        sync_profile.close()
        #client_profile.close()

# End TemplateGenerator class

if __name__ == "__main__":
    main()


#print Template (file = 'clientprofile.xml.tmpl', searchList = [{'profile':dict(cf.items('profile'))}])
#print Template (file = 'mypluginprofile.xml.tmpl', searchList = [{'plugin':dict(cf.items('clientplugin')), 'profile':dict(cf.items('profile'))}])
#print Template (file = 'mypluginprofile.xml.tmpl', searchList = [{'plugin':clientplugin}, {'profile':clientprofile}])
#print Template (file = 'sample.tmpl', searchList = [{'plugin':clientplugin}, {'profile':clientprofile}])

