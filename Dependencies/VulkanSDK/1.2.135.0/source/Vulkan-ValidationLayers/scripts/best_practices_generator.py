#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2020 The Khronos Group Inc.
# Copyright (c) 2015-2020 Valve Corporation
# Copyright (c) 2015-2020 LunarG, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Author: Mark Lobodzinski <mark@lunarg.com>

import os,re,sys,string,json
import xml.etree.ElementTree as etree
from generator import *
from collections import namedtuple
from common_codegen import *

# This is a workaround to use a Python 2.7 and 3.x compatible syntax
from io import open

class BestPracticesOutputGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 conventions = None,
                 filename = None,
                 directory = '.',
                 apiname = None,
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = None,
                 addExtensions = None,
                 removeExtensions = None,
                 emitExtensions = None,
                 sortProcedure = regSortFeatures,
                 prefixText = "",
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = True,
                 apicall = '',
                 apientry = '',
                 apientryp = '',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 0,
                 expandEnumerants = True):
        GeneratorOptions.__init__(self, conventions, filename, directory, apiname, profile,
                                  versions, emitversions, defaultExtensions,
                                  addExtensions, removeExtensions, emitExtensions, sortProcedure)
        self.prefixText      = prefixText
        self.genFuncPointers = genFuncPointers
        self.protectFile     = protectFile
        self.protectFeature  = protectFeature
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.indentFuncProto = indentFuncProto
        self.indentFuncPointer = indentFuncPointer
        self.alignFuncParam  = alignFuncParam
        self.expandEnumerants = expandEnumerants
#
# BestPracticesOutputGenerator(errFile, warnFile, diagFile)
class BestPracticesOutputGenerator(OutputGenerator):
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)
        # Commands which are not autogenerated but still intercepted
        self.no_autogen_list = [
            'vkQueueBindSparse',
            'vkAllocateMemory',
            'vkCreateValidationCacheEXT',
            'vkDestroyValidationCacheEXT',
            'vkMergeValidationCachesEXT',
            'vkGetValidationCacheDataEXT',
            ]
        self.extra_parameter_list = [
            "vkCreateShaderModule",
            "vkCreateGraphicsPipelines",
            "vkCreateComputePipelines",
            "vkAllocateDescriptorSets",
            "vkCreateRayTracingPipelinesNV",
            "vkCreateRayTracingPipelinesKHR",
            ]

        self.extension_info = dict()
    #
    # Separate content for validation source and header files
    def otwrite(self, dest, formatstring):
        if 'best_practices.h' in self.genOpts.filename and (dest == 'hdr' or dest == 'both'):
            write(formatstring, file=self.outFile)
        elif 'best_practices.cpp' in self.genOpts.filename and (dest == 'cpp' or dest == 'both'):
            write(formatstring, file=self.outFile)
    #
    # Called at beginning of processing as file is opened
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        header_file = (genOpts.filename == 'best_practices.h')
        source_file = (genOpts.filename == 'best_practices.cpp')

        if not header_file and not source_file:
            print("Error: Output Filenames have changed, update generator source.\n")
            sys.exit(1)

        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See best_practices_generator.py for modifications\n'
        self.otwrite('both', file_comment)
        # Copyright Statement
        copyright = ''
        copyright += '\n'
        copyright += '/***************************************************************************\n'
        copyright += ' *\n'
        copyright += ' * Copyright (c) 2015-2020 The Khronos Group Inc.\n'
        copyright += ' * Copyright (c) 2015-2020 Valve Corporation\n'
        copyright += ' * Copyright (c) 2015-2020 LunarG, Inc.\n'
        copyright += ' *\n'
        copyright += ' * Licensed under the Apache License, Version 2.0 (the "License");\n'
        copyright += ' * you may not use this file except in compliance with the License.\n'
        copyright += ' * You may obtain a copy of the License at\n'
        copyright += ' *\n'
        copyright += ' *     http://www.apache.org/licenses/LICENSE-2.0\n'
        copyright += ' *\n'
        copyright += ' * Unless required by applicable law or agreed to in writing, software\n'
        copyright += ' * distributed under the License is distributed on an "AS IS" BASIS,\n'
        copyright += ' * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
        copyright += ' * See the License for the specific language governing permissions and\n'
        copyright += ' * limitations under the License.\n'
        copyright += ' *\n'
        copyright += ' * Author: Mark Lobodzinski <mark@lunarg.com>\n'
        copyright += ' *\n'
        copyright += ' ****************************************************************************/\n'
        self.otwrite('both', copyright)
        self.newline()
        self.otwrite('cpp', '#include "chassis.h"')
        self.otwrite('cpp', '#include "best_practices_validation.h"')
    #
    # Now that the data is all collected and complete, generate and output the object validation routines
    def endFile(self):
        self.newline()
        # Actually write the interface to the output file.
        if (self.emit):
            self.newline()
            if self.featureExtraProtect is not None:
                prot = '#ifdef %s' % self.featureExtraProtect
                self.otwrite('both', '%s' % prot)
            if (self.featureExtraProtect is not None):
                prot = '\n#endif // %s', self.featureExtraProtect
                self.otwrite('both', prot)
            else:
                self.otwrite('both', '\n')
            ext_deprecation_data = 'const std::unordered_map<std::string, DeprecationData>  deprecated_extensions = {\n'
            for ext in sorted(self.extension_info):
                if "VK_EXT_debug_report" in ext:
                    ext_deprecation_data += '// ADD BACK AFTER LAYER TESTS SWITCH TO DEBUG_UTILS!   '
                ext_data = self.extension_info[ext]
                ext_deprecation_data += '    {"%s", {kExt%s, "%s"}},\n' % (ext, ext_data[0], ext_data[1])
            ext_deprecation_data += '};\n'
            self.otwrite('hdr', ext_deprecation_data)
        OutputGenerator.endFile(self)
    #
    # Processing point at beginning of each extension definition
    def beginFeature(self, interface, emit):
        OutputGenerator.beginFeature(self, interface, emit)
        self.featureExtraProtect = GetFeatureProtect(interface)
        ext_name = interface.attrib.get('name')
        ext_promoted = (interface.attrib.get('promotedto'))
        ext_obsoleted = interface.attrib.get('obsoletedby')
        ext_deprecated = interface.attrib.get('deprecatedby')
        if ext_promoted is not None:
           reason = 'Promoted'
           target = ext_promoted
        elif ext_obsoleted is not None:
           reason = 'Obsoleted'
           target = ext_obsoleted
        elif ext_deprecated is not None:
           reason = 'Deprecated'
           target = ext_deprecated
        else:
            return
        self.extension_info[ext_name] = [reason, target]
    #
    # Retrieve the type and name for a parameter
    def getTypeNameTuple(self, param):
        type = ''
        name = ''
        for elem in param:
            if elem.tag == 'type':
                type = noneStr(elem.text)
            elif elem.tag == 'name':
                name = noneStr(elem.text)
        return (type, name)
    #
    # Capture command parameter info needed to create, destroy, and validate objects
    def genCmd(self, cmdinfo, cmdname, alias):
        OutputGenerator.genCmd(self, cmdinfo, cmdname, alias)
        intercept = ''
        if cmdname in self.no_autogen_list:
            intercept += '// Skipping %s for autogen as it has a manually created custom function or ignored.\n' % cmdname
            self.otwrite('cpp', intercept)
            return
        cdecl=self.makeCDecls(cmdinfo.elem)[0]
        decls = self.makeCDecls(cmdinfo.elem)
        typedef = decls[1]
        typedef = typedef.split(')',1)[1]
        pre_decl = decls[0][:-1]
        pre_decl = pre_decl.split("VKAPI_CALL ")[1]
        pre_decl = pre_decl.replace(')', ',\n    VkResult                                    result)')
        if cmdname in self.extra_parameter_list:
            pre_decl = pre_decl.replace(')', ',\n    void*                                       state_data)')
        pre_decl = pre_decl.replace(')', ') {\n')
        pre_decl = 'void BestPractices::PostCallRecord' + pre_decl
        type = cdecl.split(' ')[1] 
        if type == 'VkResult':
            error_codes = cmdinfo.elem.attrib.get('errorcodes')
            success_codes = cmdinfo.elem.attrib.get('successcodes')
            success_codes = success_codes.replace('VK_SUCCESS,','')
            success_codes = success_codes.replace('VK_SUCCESS','')
            if error_codes is None and success_codes == '':
                return
            if self.featureExtraProtect is not None:
                self.otwrite('both', '#ifdef %s\n' % self.featureExtraProtect)
            func_decl = pre_decl.replace(' {',';\n');
            func_decl = func_decl.replace('BestPractices::', '')
            self.otwrite('hdr', func_decl)
            intercept += pre_decl
            params_text = ''
            params = cmdinfo.elem.findall('param')
            for param in params:
                paramtype,paramname = self.getTypeNameTuple(param)
                params_text += '%s, ' % paramname
            params_text = params_text + 'result, '
            if cmdname in self.extra_parameter_list:
                params_text += 'state_data, '
            params_text = params_text[:-2] + ');\n'
            intercept += '    ValidationStateTracker::PostCallRecord'+cmdname[2:] + '(' + params_text
            intercept += '    if (result != VK_SUCCESS) {\n'
            if error_codes is not None:
                intercept += '        static const std::vector<VkResult> error_codes = {%s};\n' % error_codes
            else:
                intercept += '        static const std::vector<VkResult> error_codes = {};\n'
            if success_codes is not None:
                intercept += '        static const std::vector<VkResult> success_codes = {%s};\n' % success_codes
            else:
                intercept += '        static const std::vector<VkResult> success_codes = {};\n'
            intercept += '        ValidateReturnCodes("%s", result, error_codes, success_codes);\n' % cmdname
            intercept += '    }\n'
            intercept += '}\n'
            self.otwrite('cpp', intercept)
            if self.featureExtraProtect is not None:
                self.otwrite('both', '#endif // %s\n' % self.featureExtraProtect)
