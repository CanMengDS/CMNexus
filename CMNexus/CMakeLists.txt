# CMakeList.txt: CMNexus 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
add_executable (CMNexus "CMNexus.cpp" "include/CMNexus.h"  "include/Abstract/AbstractIOCompletionPortServer.h" "src/AbstractIOCompletionPortServer.cpp" "include/CMNetDefs.h" "include/Abstract/AbstractCommondSolve.h"  "include/CommondSolve.h" "src/CommondSolve.cpp" "include/DataHeaderInit.h" "src/DataHeaderInit.cpp" "include/CMThreadPool.hpp" "src/CMThreadPool.cpp" "include/OverlappedPerIOPool.h" "src/OverlappedPerIOPool.cpp" "include/CMFileSolve.h" "src/CMFileSolve.cpp" "include/CMTcpServer.h" "src/CMTcpServer.cpp")


target_include_directories(
    CMNexus          # 绑定到你的可执行目标（必须和 add_executable 的目标名称一致）
    PRIVATE          # 作用域（可执行项目用 PRIVATE，库项目用 PUBLIC）
    ${CMAKE_CURRENT_SOURCE_DIR}/include  # 头文件目录路径
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMNexus PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
