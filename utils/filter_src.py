Import("env")

# This filter script is needed to exclude some of RemoteDebug source code files which are not compatible with new version
# of Arduino library. To use the external WebSockets library from Platformio library repository and ignore the files 
# included by the RemoteDebug library author (those files are located in .pio/libdeps/*/RemoteDebug/utility foleder), 
# we have to use filter script. For details see here:
# https://community.platformio.org/t/solved-exclude-directory-from-imported-external-library/10052/8

def skip_from_build(node):
    """
    `node.name` - a name of File System Node
    `node.get_path()` - a relative path
    `node.get_abspath()` - an absolute path
        to ignore file from a build process, just return None
    """
    if "RemoteDebug/utility" in node.get_path():
        # Return None for exclude
        return None

    return node

# Register callback
env.AddBuildMiddleware(skip_from_build, "*")