# Sources
set(DB_SOURCE_DIR src)
set(DB_SOURCES
  ${DB_SOURCE_DIR}/file.cc
  ${DB_SOURCE_DIR}/bpt.cc
  ${DB_SOURCE_DIR}/debug.cc
  ${DB_SOURCE_DIR}/newbpt.cc
  ${DB_SOURCE_DIR}/buffer.cc
  ${DB_SOURCE_DIR}/page.cc
  ${DB_SOURCE_DIR}/lock_table.cc
  ${DB_SOURCE_DIR}/trx.cc
  # Add your sources here
  # ${DB_SOURCE_DIR}/foo/bar/your_source.cc
  )

# Headers
set(DB_HEADER_DIR include)
set(DB_HEADERS
  ${DB_HEADER_DIR}/file.h
  ${DB_HEADER_DIR}/bpt.h
  ${DB_HEADER_DIR}/debug.h
  ${DB_HEADER_DIR}/newbpt.h
  ${DB_HEADER_DIR}/buffer.h
  ${DB_HEADER_DIR}/page.h
  ${DB_HEADER_DIR}/lock_table.h
  ${DB_HEADER_DIR}/trx.h
  # Add your headers here
  # ${DB_HEADER_DIR}/foo/bar/your_header.h
  )

add_library(db STATIC ${DB_HEADERS} ${DB_SOURCES})

target_include_directories(db
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${DB_HEADER_DIR}"
  )

