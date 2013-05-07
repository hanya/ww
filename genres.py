#!/usr/bin/env python

import os, os.path

# Generates resource file from each po file.
# And also other configuration stuff too.

desc_h = """<?xml version='1.0' encoding='UTF-8'?>
<description xmlns="http://openoffice.org/extensions/description/2006"
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:d="http://openoffice.org/extensions/description/2006">
<identifier value="mytools.calc.CppWatchingWindow" />
<version value="{VERSION}" />
<dependencies>
<OpenOffice.org-minimal-version value="3.4" d:name="OpenOffice.org 3.4" />
</dependencies>
<!--
<registration>
<simple-license accept-by="admin" default-license-id="this" suppress-on-update="true" suppress-if-required="true">
<license-text xlink:href="LICENSE" lang="en" license-id="this" />
</simple-license>
</registration>
-->
<display-name>
{NAMES}
</display-name>
<extension-description>
{DESCRIPTIONS}
</extension-description>
<!--
<update-information>
<src xlink:href="https://raw.github.com/hanya/WatchingWindow/master/files/WatchingWindow.update.xml"/>
</update-information>
-->
</description>"""

update_feed = """<?xml version="1.0" encoding="UTF-8"?>
<description xmlns="http://openoffice.org/extensions/update/2006" 
xmlns:xlink="http://www.w3.org/1999/xlink"
xmlns:d="http://openoffice.org/extensions/description/2006">
<identifier value="mytools.calc.CppWatchingWindow" />
<version value="{VERSION}" />
<dependencies>
<d:OpenOffice.org-minimal-version value="3.4" d:name="OpenOffice.org 3.4" />
</dependencies>
<update-download>
<src xlink:href="https://raw.github.com/hanya/WatchingWindow/master/files/WatchingWindow-{VERSION}.oxt"/>
</update-download>
</description>
"""

optionsdialog_xcu = """<?xml version="1.0" encoding="UTF-8"?>
<oor:component-data
oor:name="OptionsDialog"
oor:package="org.openoffice.Office"
xmlns:oor="http://openoffice.org/2001/registry"
xmlns:xs="http://www.w3.org/2001/XMLSchema"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <node oor:name="Nodes">
    <node oor:name="Calc" oor:op="fuse">
      <node oor:name="Leaves">
        <node oor:name="mytools.calc.CppWatchingWindow_01" oor:op="fuse">
          <prop oor:name="Id">
            <value>mytools.calc.CppWatchingWindow</value>
          </prop>
          <prop oor:name="Label">
            {LABEL}
          </prop>
          <prop oor:name="OptionsPage">
            <value>%origin%/dialogs/Settings.xdl</value>
          </prop>
          <prop oor:name="EventHandlerService">
            <value>mytools.config.CppWatchingWindowOptions</value>
          </prop>
        </node>
      </node>
    </node>
  </node>
</oor:component-data>"""

calcwindowstate_xcu = """<?xml version='1.0' encoding='UTF-8'?>
<oor:component-data 
  xmlns:oor="http://openoffice.org/2001/registry" 
  xmlns:xs="http://www.w3.org/2001/XMLSchema" 
  oor:package="org.openoffice.Office.UI" 
  oor:name="CalcWindowState">
  <node oor:name="UIElements">
    <node oor:name="States">
      <node oor:name="private:resource/toolpanel/mytools.calc/WatchingWindow" oor:op="replace">
        <prop oor:name="UIName">
          {UINAME}
        </prop>
        <prop oor:name="ImageURL">
          <value>vnd.sun.star.extension://mytools.calc.CppWatchingWindow/icons/ww_24.png</value>
        </prop>
      </node>
    </node>
  </node>
</oor:component-data>"""


sidebar_xcu = """<?xml version="1.0" encoding="UTF-8"?>
<oor:component-data
oor:name="Sidebar"
oor:package="org.openoffice.Office.UI"
xmlns:oor="http://openoffice.org/2001/registry"
xmlns:xs="http://www.w3.org/2001/XMLSchema"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <node oor:name="Content">
    <node oor:name="DeckList">
      <node oor:name="WatchingDeck" oor:op="replace">
        <prop oor:name="Title" oor:type="xs:string">
          {TITLE}
        </prop>
        <prop oor:name="Id" oor:type="xs:string">
          <value>WatchingDeck</value>
        </prop>
        <prop oor:name="IconURL" oor:type="xs:string">
          <value>vnd.sun.star.extension://mytools.calc.CppWatchingWindow/icons/ww_24.png</value>
        </prop>
        <prop oor:name="HighContrastIconURL" oor:type="xs:string">
          <value>vnd.sun.star.extension://mytools.calc.CppWatchingWindow/iconsh/ww_24.png</value>
        </prop>
        <prop oor:name="ContextList">
          <value oor:separator=";">
            Calc, any, visible ;
          </value>
        </prop>
        <prop oor:name="OrderIndex" oor:type="xs:int">
          <value>500</value>
        </prop>
      </node>
    </node>
    <node oor:name="PanelList">
      <node oor:name="WatchingWindowPanel" oor:op="replace">
        <prop oor:name="Title" oor:type="xs:string">
          {TITLE}
        </prop>
        <prop oor:name="TitleBarIsOptional" oor:type="xs:boolean">
          <value>true</value>
        </prop>
        <prop oor:name="Id" oor:type="xs:string">
          <value>WatchingWindowPanel</value>
        </prop>
        <prop oor:name="DeckId" oor:type="xs:string">
          <value>WatchingDeck</value>
        </prop>
        <prop oor:name="ContextList">
          <value oor:separator=";">
            Calc, any, visible ;
          </value>
        </prop>
        <prop oor:name="ImplementationURL" oor:type="xs:string">
          <value>private:resource/toolpanel/mytools.calc/WatchingWindow</value>
        </prop>
        <prop oor:name="OrderIndex" oor:type="xs:int">
          <value>100</value>
        </prop>
      </node>
    </node>
  </node>
</oor:component-data>
"""



def genereate_description(d, out_dir):
    version = read_version()
    
    names = []
    for lang, v in d.iteritems():
        name = v["id.label.ww"]
        names.append("<name lang=\"{LANG}\">{NAME}</name>".format(LANG=lang, NAME=name.encode("utf-8")))
    
    descs = []
    for lang, v in d.iteritems():
        desc = v["id.extension.description"]
        with open(os.path.join(out_dir, "descriptions/desc_{LANG}.txt").format(LANG=lang), "w") as f:
            f.write(desc.encode("utf-8"))
        descs.append("<src lang=\"{LANG}\" xlink:href=\"descriptions/desc_{LANG}.txt\"/>".format(LANG=lang))
    
    return desc_h.format(
        VERSION=version, NAMES="\n".join(names), DESCRIPTIONS="\n".join(descs))


def read_version():
    version = ""
    with open("VERSION") as f:
        version = f.read().strip()
    return version


class XCUDataTemplated(object):
    
    TEMPLATE = ""
    
    def __init__(self):
        self.template = None
        self._replacements = {}
    
    def add_value_for_locales(self, replaced_key, k, d):
        a = []
        locales = list(d.iterkeys())
        locales.sort()
        for lang in locales:
            _d = d[lang]
            a.append("<value xml:lang=\"{LANG}\">{VALUE}</value>".format(VALUE=_d[k].encode("utf-8"), LANG=lang))
        self._add_replacement(replaced_key, "\n".join(a))
    
    def _add_replacement(self, key, value):
        self._replacements[key] = value
    
    def _format(self):
        return self.template.format(**self._replacements)
    
    def generate(self, d):
        self.template = self.__class__.TEMPLATE
        self._generate(d)
        return self._format()


class CalcWindowStateXCU(XCUDataTemplated):
    
    TEMPLATE = calcwindowstate_xcu
    
    def _generate(self, d):
        self.add_value_for_locales("UINAME", "id.label.ww", d)


class SidebarXCU(XCUDataTemplated):
    
    TEMPLATE = sidebar_xcu
    
    def _generate(self, d):
        self.add_value_for_locales("TITLE", "id.label.ww", d)


class OptionsDialogXCU(XCUDataTemplated):
    
    TEMPLATE = optionsdialog_xcu
    
    def _generate(self, d):
        self.add_value_for_locales("LABEL", "id.label.ww", d)


def extract(d, locale, lines):
    msgid = msgstr = id = ""
    for l in lines:
        #if l[0] == "#":
        #    pass
        if l[0:2] == "#,":
            pass
        elif l[0:2] == "#:":
            id = l[2:].strip()
        if l[0] == "#":
            continue
        elif l.startswith("msgid"):
            msgid = l[5:]
        elif l.startswith("msgstr"):
            msgstr = l[6:].strip()
            #print(id, msgid, msgstr)
            if msgstr and id:
                d[id] = msgstr[1:-1].decode("utf-8").replace('\\"', '"')
        _l = l.strip()
        if not _l:
            continue


def as_resource(d):
    lines = []
    
    for k, v in d.iteritems():
        cs = []
        for c in v:
            a = ord(c)
            if a > 0x7f:
                cs.append("\\u%04x" % a)
            else:
                cs.append(c)
        lines.append("%s=%s" % (k, "".join(cs)))
    lines.sort()
    return "\n".join(lines)


def write_resource(res_path, d):
    lines = as_resource(d)
    with open(res_path, "w") as f:
        f.write("# comment\n")
        f.write(lines.encode("utf-8"))

def write_update_feed():
    version = read_version()
    s = update_feed.format(VERSION=version)
    with open(os.path.join(".", "files", "WatchingWindow.update.xml"), "w") as f:
        f.write(s.encode("utf-8"))

def main():
    out_dir = "gen"
    
    prefix = "strings_"
    res_dir = "resources"
    
    locales = {}
    
    po_dir = os.path.join(".", "po")
    for po in os.listdir(po_dir):
        if po.endswith(".po"):
            locale = po[:-3]
            try:
                lines = open(os.path.join(po_dir, po)).readlines()
            except:
                print("%s can not be opened")
            d = {}
            extract(d, locale, lines)
            locales[locale] = d
    
    resources_dir = os.path.join(".", out_dir, res_dir)
    
    for locale, d in locales.iteritems():
        write_resource(os.path.join(resources_dir, 
            "%s%s.properties" % (prefix, locale.replace("-", "_"))), d)
    with open(os.path.join(resources_dir, "%sen_US.default" % prefix), "w") as f:
        pass
    
    def store_xcu_data(klass, out):
        with open(os.path.join(out_dir, out), "w") as f:
            f.write(klass().generate(locales))
    
    store_xcu_data(CalcWindowStateXCU, "CalcWindowState.xcu")
    store_xcu_data(SidebarXCU, "Sidebar.xcu")
    store_xcu_data(OptionsDialogXCU, "OptionsDialog.xcu")
    
    s = genereate_description(locales, out_dir)
    with open(os.path.join(out_dir, "description.xml"), "w") as f:
        f.write(s)#.encode("utf-8"))
    
    write_update_feed()


if __name__ == "__main__":
    main()
