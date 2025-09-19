ADD_LIBRARY(squelch
  automatizer.cc
  condition.cc
  condition_metadata.cc
  cursor.cc
  object_status.cc 
  rule.cc
  tree_printer.cc
)
TARGET_LINK_LIBRARIES(squelch PUBLIC ${LIBS})
