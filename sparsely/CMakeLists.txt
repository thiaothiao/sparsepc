cmake_minimum_required(VERSION 4.1)

set(appName "Sparsely")
set(exeName "sparsely")
set(appVersion "0.1.0")
set(appTitle "SPARSELY Software - SPARSE principal component LaboratorY")
set(appVendor "Inulo")
set(appDomain "?.com")
string(TIMESTAMP buildTime "%Y-%m-%d %H:%M:%S")

project(${exeName}
        VERSION ${appVersion}
        DESCRIPTION ${appTitle}
        LANGUAGES CXX
)
		
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

include(../cmake/tools.cmake)
include(../cmake/CPM.cmake)

# Generate the header file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/infos.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/infos.h"
    @ONLY
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT TARGET Eigen3::Eigen)
        CPMAddPackage(
          NAME Eigen3
          GITLAB_REPOSITORY libeigen/eigen
          GIT_TAG 5.0.1
          # Eigen's CMakelists are not intended for library use
          DOWNLOAD_ONLY YES
          #OPTIONS "EIGEN_BUILD_DOC OFF" "BUILD_TESTING OFF"
        )

    if(Eigen3_ADDED)
        add_library(Eigen3::Eigen INTERFACE IMPORTED)
        target_include_directories(Eigen3::Eigen INTERFACE ${Eigen3_SOURCE_DIR})
    endif()
endif()

if(NOT TARGET Spectra::Spectra)
    CPMAddPackage(
      NAME Spectra
      GITHUB_REPOSITORY yixuan/spectra
      GIT_TAG v1.2.0
      DOWNLOAD_ONLY YES
    )

    if(Spectra_ADDED)
        add_library(Spectra::Spectra INTERFACE IMPORTED)
        target_include_directories(Spectra::Spectra INTERFACE ${Spectra_SOURCE_DIR}/include)
    endif()
endif()

CPMAddPackage(
	NAME sparsepc
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Xml Svg REQUIRED)

CPMAddPackage(
    NAME JKQTPlotter6
    GITHUB_REPOSITORY jkriege2/JKQtPlotter
    GIT_TAG master # Or a specific version/commit
	COMPONENTS JKQTPlotter6 JKQTCommon6 JKQTMath6 JKQTMathText6
	FIND_PACKAGE_ARGUMENTS REQUIRED
	OPTIONS 
		"JKQtPlotter_BUILD_FORCE_NO_PRINTER_SUPPORT ON" 
		"JKQtPlotter_BUILD_TESTS OFF" "JKQtPlotter_BUILD_EXAMPLES OFF" 
		"JKQtPlotter_BUILD_LIB_JKQTFASTPLOTTER OFF"
)

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB apprccs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc)

if(WIN32)
	file(GLOB appiconrcs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images/*.rc)
    enable_language(RC)
elseif(APPLE)
	set(appiconrcs "${CMAKE_CURRENT_SOURCE_DIR}/images/app.icns")
	set(MACOSX_BUNDLE_ICON_FILE "app.icns")
	set_source_files_properties(${appiconrcs} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

qt_standard_project_setup()
qt_add_resources(sources ${apprccs})
qt_add_executable(${exeName} ${sources} ${appiconrcs} ${headers})

if(WIN32)
	set_target_properties(${exeName} PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
	set_target_properties(${exeName} PROPERTIES 
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.inulo.sparsely"
            MACOSX_BUNDLE_BUNDLE_NAME ${exeName}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${appVersion}
            MACOSX_BUNDLE_LONG_VERSION_STRING ${appVersion}
	)	
endif()

target_compile_definitions(${exeName} PRIVATE _CRT_SECURE_NO_WARNINGS)
target_compile_definitions(${exeName} PRIVATE _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)

target_link_libraries(${exeName} PRIVATE Eigen3::Eigen)

target_link_libraries(${exeName} PRIVATE Spectra::Spectra)

target_link_libraries(${exeName} PRIVATE sparsepc::sparsepc)

target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTPlotter6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTCommon6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMath6)
target_link_libraries(${exeName} PRIVATE JKQTPlotter6::JKQTMathText6)

target_link_libraries(${exeName} PRIVATE Qt6::Core)
target_link_libraries(${exeName} PRIVATE Qt6::Widgets)
target_link_libraries(${exeName} PRIVATE Qt6::Gui)
target_link_libraries(${exeName} PRIVATE Qt6::Xml)
target_link_libraries(${exeName} PRIVATE Qt6::Svg)

include(CPackComponent)
cpack_add_component(${appName}
    DISPLAY_NAME ${appName}
    DESCRIPTION ${appTitle}
    REQUIRED
)

if(WIN32)# grab some dependencies
    install(FILES   $<TARGET_FILE:JKQTPlotter6::JKQTPlotter6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTCommon6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTMath6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTMathText6>
                    $<TARGET_FILE:Qt6::Xml>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT ${appName}
    )
elseif(APPLE)
    install(FILES   $<TARGET_FILE:JKQTPlotter6::JKQTPlotter6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTCommon6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTMath6>
                    $<TARGET_FILE:JKQTPlotter6::JKQTMathText6>
        DESTINATION "${exeName}.app/Contents/Frameworks"
        COMPONENT ${appName}
    )
else()# assuming linux. grab icons
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/images/icon.svg"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons"
        COMPONENT "${appName}"
    )
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/sparsely.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
        COMPONENT "${appName}"
    )
endif()

if(APPLE)
    set(SIGNING_IDENTITY "-")#"Developer ID Application: Your Company (ID)") # Define your signing identity
    set(DEPLOY_OPTIONS "-codesign=${SIGNING_IDENTITY}" "-hardened-runtime" "-timestamp")
else()
    set(DEPLOY_OPTIONS "--no-translations")
endif()

qt6_generate_deploy_app_script(
    TARGET ${exeName}
    OUTPUT_SCRIPT deploy_script
    DEPLOY_TOOL_OPTIONS ${DEPLOY_OPTIONS}
)

if (APPLE)
    install(TARGETS ${exeName}
        BUNDLE DESTINATION .
        COMPONENT ${appName}
    )
else()
    install(TARGETS ${exeName}
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"    # 'bin'
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"    # 'lib' or 'lib64'
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"    # static libs
        COMPONENT "${appName}"
    )
endif()

install(SCRIPT ${deploy_script}
    COMPONENT "${appName}"
)

set(CPACK_GENERATOR "IFW")
set(CPACK_IFW_VERBOSE ON)
set(CPACK_PACKAGE_NAME "${appName}")
set(CPACK_PACKAGE_VENDOR "${appVendor}")
set(CPACK_PACKAGE_CONTACT "mamadou.thiao@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${appTitle}")
set(CPACK_PACKAGE_VERSION "${appVersion}")
#set(CPACK_COMPONENTS_ALL ${appName})#Application Runtime Documentation)
#set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
#set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
#set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
#set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})
#set(CPACK_VERBATIM_VARIABLES YES)

# if we not call the line below, groups won't show properlly in the GUI installer
set(CPACK_COMPONENTS_GROUPING IGNORE)

# ignore directories and files
set(CPACK_SOURCE_IGNORE_FILES
  "/.git/"
  "/.vscode/"
  "/build/"
  "/CMakeCache.txt"
  "/CMakeFiles/"
  "/cmake_install.cmake"
  "/CPackConfig.cmake"
)

set(CPACK_IFW_PACKAGE_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/images/inulo.png")
#set(CPACK_IFW_PACKAGE_BANNER ${CMAKE_CURRENT_SOURCE_DIR}/images/banner.png)
#set(CPACK_IFW_PACKAGE_WINDOW_ICON ${CMAKE_CURRENT_SOURCE_DIR}/images/window.png)
#set(CPACK_IFW_PACKAGE_WINDOWS_LOGO ${CMAKE_CURRENT_SOURCE_DIR}/images/windows.png)
set(CPACK_IFW_PACKAGE_TITLE "${appTitle}")
set(CPACK_IFW_PACKAGE_PUBLISHER "${appVendor}")
set(CPACK_IFW_PRODUCT_URL "${appDomain}")

#set(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDir@/MonApplication")

#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE.txt")

#set(CPACK_IFW_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/installer/configuration")
#set(CPACK_IFW_PACKAGE_REPOSITORIES_DEFAULT "remote")

# create a more memorable name for the maintenance tool (used for uninstalling the package)
set(CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME "${appName}_MaintenanceTool")
#set(CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_INI_FILE ${CMAKE_CURRENT_SOURCE_DIR}/configurations/maintenance_tool.ini)

set(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern") # customise the theme if required

# adjust the default size
set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_HEIGHT 400)
#set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_WIDTH ?)

#set(CPACK_IFW_PACKAGE_WATERMARK ${CMAKE_CURRENT_SOURCE_DIR}/images/watermark.png)

# 3. Set the main installer executable icon
if(WIN32)
    set(CPACK_IFW_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/app.ico")
    set(CPACK_RESOURCE_FILE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/uninstall.ico")
elseif(APPLE)
    set(CPACK_IFW_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/app.icns")
endif()

#set(CPACK_IFW_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/installer/configuration/config.xml)

# Link a custom control script to handle the installation logic
#set(CPACK_IFW_PACKAGE_CONTROL_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/installscript.qs")

include(CPack)
include(CPackIFW)

# Optional: Add IFW specific information
cpack_ifw_configure_component(${appName}
    NAME "com.${appVendor}.${appName}"
    DISPLAY_NAME "${appName}"
    DESCRIPTION "${appTitle}"
    VERSION "${appVersion}"
    RELEASE_DATE "${buildTime}"
    SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/installscript.qs"
)
