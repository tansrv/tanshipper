cmake_minimum_required(VERSION 2.8)

project(tanshipper)

set(INSTALL_PATH /usr/local/tanshipper)

aux_source_directory(. SRCS)
add_executable(tanshipper ${SRCS})

target_link_libraries(tanshipper pthread crypto ssl)

install(TARGETS tanshipper DESTINATION ${INSTALL_PATH}/sbin)
install(DIRECTORY DESTINATION ${INSTALL_PATH}/logs)
install(FILES ../conf/tanshipper.conf DESTINATION /etc)
install(FILES ../misc/tanshipper PERMISSIONS OWNER_EXECUTE DESTINATION /etc/init.d)
