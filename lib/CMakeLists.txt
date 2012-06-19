INSTALL(FILES
  module.lua
  DESTINATION ${DEFAULT_PATH}
)
INSTALL(DIRECTORY 
  apex
  cmov
  data
  dngn
  edit
  file
  help
  info
  mods
  note
  plot
  pref
  save
  user
  xtra
  DESTINATION ${DEFAULT_PATH}
  PATTERN "*.raw" EXCLUDE
  PATTERN "delete.me" EXCLUDE
  )
