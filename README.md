## Building the SU2 Grid Import Plugin

These instructions have been updated for Plugin SDK v1.0 R11 for Pointwise 18.1.
See previous versions of this README if using older SDK distibutions.

To build the SU2 grid import plugin you must integrate the SU2 source 
code into your local PluginSDK installation. These instructions assume you have 
already downloaded, installed, and [verified][SDKdocs] the 
[Pointwise Plugin SDK][SDKdownload]. Be sure the SDK you are using is 
compatible with the version of Pointwise you are targeting.

To integrate the SU2 source code, you must first create a plugin project 
in your SDK using the steps shown below. You may replace `MySU2` with any 
unique name you choose. Even though the SDK builds with C++ compilers, the 
SU2 plugin uses the C-style entry points. Consequently, you *must* use the 
`-c` option with `mkplugin` below.

If you plan on building the plugin for multiple platforms, I suggest that you 
place the SDK on a network drive. That way you can build the SU2 plugin 
using the same SDK installation. You only need to create the plugin project one 
time. Once a plugin project is created, it can be accessed and built on all 
platforms supported by Pointwise.

In the sections below, the `PluginSDK` folder is shorthand for the full path 
to the `PluginSDK` folder of your SDK installation. That is, `PluginSDK` 
is shorthand for `\some\long\path\to\your\install\of\PluginSDK`.

### Creating the SU2 Plugin Project on Unix and Mac OS/X
   * `% cd PluginSDK`
   * `% mkplugin -grdp -c MySU2`
   * `% make GrdpMySU2-dr`

### Building the SU2 Plugin Project on Windows

#### At the Command Prompt
 * `C:> cd PluginSDK`
 * `C:> mkplugin -grdp -c MySU2`
 * `C:> exit`

#### In Visual Studio 2015

 * Choose the *File &gt; Open &gt; Project/Solution...* menu
 * Select the file `PluginSDK\PluginSDK_vs2015.sln`
 * In the *Solution Explorer* window
  * Right-click on the *Plugins* folder
  * Choose *Add &gt; Existing Project...* in the popup menu
  * Select `PluginSDK\src\plugins\GrdpMySU2\GrdpMySU2.vcxproj`
  * Right-click on the *GrdpMySU2* project
  * Choose *Build* in the popup menu

If all goes well above, you should be able to build the GrdpMySU2 plugin 
without errors or warnings. The plugin does not do anything yet. Building at this 
point was done to be sure we are ready to integrate the SU2 source code 
from this repository.

### Integrating the SU2 Plugin Source Code into Your SDK

Once the `GrdpMySU2` project is created and builds correctly as 
explained above, integrate the source code from this repository into 
your project by copying the repository files into the 
`PluginSDK\src\plugins\GrdpMySU2` folder. This overwrites the files 
generated by `mkplugin`.

Edit the file `PluginSDK\src\plugins\GrdpMySU2\rtGrdpInitItems.h`. 
Change the plugin's name from

> ```
> "SU2",     /* const char *name */
> ```

to

> ```
> "MySU2",   /* const char *name */
> ```

Save the file.

You can now build the `GrdpMySU2` plugin on your platform. If all 
builds correctly, you now have a MySU2 grid import plugin that is 
functionally equivalent to the SU2 plugin distributed with Pointwise. You 
are now ready to make your changes to the GrdpMySU2 plugin.

### Closing Thoughts

See the [Building a CAE Plugin][SDKbuild] for information on configuring your 
SDK build environment.

If you plan on publicly releasing source or binary builds of your version of 
the SU2 grid importer plugin, you need to be careful about your choice of 
site id, site group name, plugin id, and plugin name. See documentation for 
[site.h][SDKsite.H] for details. Your plugin may not load correctly into 
Pointwise if the values you choose conflict with other Pointwise plugins.

Being open source, we would like to see any enhancements or bug fixes you make 
to the SU2 plugin integrated back into the main Pointwise build. Please do 
not hesitate to send us git pull requests for your changes. We will evaluate 
your changes and integrate them as appropriate.


[SDKdownload]: http://www.pointwise.com/plugins/html/index.html#sdk_downloads
[SDKdocs]: http://www.pointwise.com/plugins
[SDKsite.H]: http://www.pointwise.com/plugins/html/d6/d89/site_8h.html
[SDKbuild]: http://www.pointwise.com/plugins/html/da/dde/build_cae_plugin.html
