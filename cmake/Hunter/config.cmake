#[[
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs

   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
]]

include(hunter_cmake_args)

hunter_cmake_args(
        abseil
        CMAKE_ARGS
        ABSL_PROPAGATE_CXX_STD=ON
        ABSL_ENABLE_INSTALL=OFF
        ABSL_RUN_TESTS=OFF
)

hunter_config(
        Boost
        URL https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0.tar.bz2
        SHA1 898469f1ae407f5cbfca84f63ad602962eebf4cc
)

hunter_config(
        OpenSSL
        URL https://github.com/openssl/openssl/archive/OpenSSL_1_1_1t.tar.gz
        SHA1 34ea65451f7fc4625f31ba50f89b3fbea12f13f3
)