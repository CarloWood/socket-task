m4_if(cwm4_submodule_dirname, [], [m4_append_uniq([CW_SUBMODULE_SUBDIRS], cwm4_submodule_basename, [ ])])
m4_append_uniq([CW_SUBMODULE_CONFIG_FILES], cwm4_quote(cwm4_submodule_path[/Makefile]), [ ])

# Add socket-task dependencies.
m4_if(cwm4_submodule_dirname, [], [m4_append_uniq_w([CW_SUBMODULE_SUBDIRS], [statefultask evio resolver-task], [ ])])
