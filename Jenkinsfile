#!/usr/bin/env groovy

dgl_linux_libs = "build/libdgl.so, build/runUnitTests, python/dgl/_ffi/_cy3/core.cpython-36m-x86_64-linux-gnu.so"
// Currently DGL on Windows is not working with Cython yet
dgl_win64_libs = "build\\dgl.dll, build\\runUnitTests.exe"

def init_git() {
  sh "rm -rf *"
  checkout scm
  sh "git submodule update --recursive --init"
}

def init_git_win64() {
  checkout scm
  bat "git submodule update --recursive --init"
}

// pack libraries for later use
def pack_lib(name, libs) {
  echo "Packing ${libs} into ${name}"
  stash includes: libs, name: name
}

// unpack libraries saved before
def unpack_lib(name, libs) {
  unstash name
  echo "Unpacked ${libs} from ${name}"
}

def build_dgl_linux(dev, release) {
  init_git()
  sh "bash tests/scripts/build_dgl.sh ${dev}"
  pack_lib("dgl-${dev}-linux-${release}", dgl_linux_libs)
}

def build_dgl_win64(dev, release) {
  /* Assuming that Windows slaves are already configured with MSBuild VS2017,
   * CMake and Python/pip/setuptools etc. */
  init_git_win64()
  if (dev == "gpu") {
    bat "CALL tests\\scripts\\build_dgl.bat ON ${release}"
  } else {
    bat "CALL tests\\scripts\\build_dgl.bat OFF ${release}"
  }
  pack_lib("dgl-${dev}-win64-${release}", dgl_win64_libs)
}

def cpp_unit_test_linux(release) {
  init_git()
  unpack_lib("dgl-cpu-linux-${release}", dgl_linux_libs)
  sh "bash tests/scripts/task_cpp_unit_test.sh"
}

def cpp_unit_test_win64(release) {
  init_git_win64()
  unpack_lib("dgl-cpu-win64-${release}", dgl_win64_libs)
  bat "CALL tests\\scripts\\task_cpp_unit_test.bat"
}

def unit_test_linux(backend, dev, release) {
  init_git()
  unpack_lib("dgl-${dev}-linux-${release}", dgl_linux_libs)
  timeout(time: 10, unit: 'MINUTES') {
    sh "bash tests/scripts/task_unit_test.sh ${backend} ${dev}"
  }
}

def unit_test_win64(backend, dev, release) {
  init_git_win64()
  unpack_lib("dgl-${dev}-win64-${release}", dgl_win64_libs)
  timeout(time: 10, unit: 'MINUTES') {
    bat "CALL tests\\scripts\\task_unit_test.bat ${backend}"
  }
}

def example_test_linux(backend, dev, release) {
  init_git()
  unpack_lib("dgl-${dev}-linux-${release}", dgl_linux_libs)
  timeout(time: 20, unit: 'MINUTES') {
    sh "bash tests/scripts/task_example_test.sh ${dev}"
  }
}

def example_test_win64(backend, dev, release) {
  init_git_win64()
  unpack_lib("dgl-${dev}-win64-${release}", dgl_win64_libs)
  timeout(time: 20, unit: 'MINUTES') {
    bat "CALL tests\\scripts\\task_example_test.bat ${dev}"
  }
}

def tutorial_test_linux(backend, release) {
  init_git()
  unpack_lib("dgl-cpu-linux-${release}", dgl_linux_libs)
  timeout(time: 20, unit: 'MINUTES') {
    sh "bash tests/scripts/task_${backend}_tutorial_test.sh"
  }
}


pipeline {
  agent any
  stages {
    stage("Lint Check") {
      agent { 
        docker {
          label "linux-c52x-node"
          image "dgllib/dgl-ci-lint"  
          alwaysPull true
        }
      }
      steps {
        init_git()
        sh "bash tests/scripts/task_lint.sh"
      }
      post {
        always {
          cleanWs disableDeferredWipeout: true, deleteDirs: true
        }
      }
    }
    stage("Build") {
      parallel {
        stage("CPU Build") {
          agent { 
            docker {
              label "linux-c52x-node"
              image "dgllib/dgl-ci-cpu:conda" 
              alwaysPull true
            }
          }
          steps {
            build_dgl_linux("cpu", "Release")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("CPU Build (Debug)") {
          agent { 
            docker {
              label "linux-c52x-node"
              image "dgllib/dgl-ci-cpu:conda" 
              alwaysPull true
            }
          }
          steps {
            build_dgl_linux("cpu", "Debug")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("GPU Build") {
          agent {
            docker {
              label "linux-c52x-node"
              image "dgllib/dgl-ci-gpu:conda"
              args "-u root"
              alwaysPull true
            }
          }
          steps {
            // sh "nvidia-smi"
            build_dgl_linux("gpu", "Release")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("GPU Build (Debug)") {
          agent {
            docker {
              label "linux-c52x-node"
              image "dgllib/dgl-ci-gpu:conda"
              args "-u root"
              alwaysPull true
            }
          }
          steps {
            // sh "nvidia-smi"
            build_dgl_linux("gpu", "Debug")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("CPU Build (Win64)") {
          // Windows build machines are manually added to Jenkins master with
          // "windows" label as permanent agents.
          agent { label "windows" }
          steps {
            build_dgl_win64("cpu", "Release")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("CPU Build (Win64) (Debug)") {
          // Windows build machines are manually added to Jenkins master with
          // "windows" label as permanent agents.
          agent { label "windows" }
          steps {
            build_dgl_win64("cpu", "Debug")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("GPU Build (Win64)") {
          // Windows build machines are manually added to Jenkins master with
          // "windows" label as permanent agents.
          agent { label "windows" }
          steps {
            build_dgl_win64("gpu", "Release")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("GPU Build (Win64) (Debug)") {
          // Windows build machines are manually added to Jenkins master with
          // "windows" label as permanent agents.
          agent { label "windows" }
          steps {
            build_dgl_win64("gpu", "Debug")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
      }
    }
    stage("Test") {
      parallel {
        stage("C++ CPU") {
          agent { 
            docker { 
              label "linux-c52x-node"
              image "dgllib/dgl-ci-cpu:conda"
              alwaysPull true
            }
          }
          steps {
            cpp_unit_test_linux("Release")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("C++ CPU (Debug)") {
          agent { 
            docker { 
              label "linux-c52x-node"
              image "dgllib/dgl-ci-cpu:conda"
              alwaysPull true
            }
          }
          steps {
            cpp_unit_test_linux("Debug")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("C++ CPU (Win64)") {
          agent { label "windows" }
          steps {
            cpp_unit_test_win64("Release")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("C++ CPU (Win64) (Debug)") {
          agent { label "windows" }
          steps {
            cpp_unit_test_win64("Debug")
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Tensorflow CPU") {
          agent { 
            docker {
              label "linux-c52x-node"
              image "dgllib/dgl-ci-cpu:conda" 
              alwaysPull true
            }
          }
          stages {
            stage("Unit test") {
              steps {
                unit_test_linux("tensorflow", "cpu", "Release")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Tensorflow GPU") {
          agent { 
            docker { 
              label "linux-gpu-node"
              image "dgllib/dgl-ci-gpu:conda" 
              args "--runtime nvidia"
              alwaysPull true
            }
          }
          stages {
            stage("Unit test") {
              steps {
                unit_test_linux("tensorflow", "gpu", "Release")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Torch CPU") {
          agent { 
            docker {
              label "linux-c52x-node"
              image "dgllib/dgl-ci-cpu:conda" 
              alwaysPull true
            }
          }
          stages {
            stage("Unit test") {
              steps {
                unit_test_linux("pytorch", "cpu", "Release")
              }
            }
            stage("Example test") {
              steps {
                example_test_linux("pytorch", "cpu", "Release")
              }
            }
            stage("Tutorial test") {
              steps {
                tutorial_test_linux("pytorch", "Release")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Torch CPU (Debug)") {
          agent { 
            docker {
              label "linux-c52x-node"
              image "dgllib/dgl-ci-cpu:conda" 
              alwaysPull true
            }
          }
          stages {
            stage("Unit test") {
              steps {
                unit_test_linux("pytorch", "cpu", "Debug")
              }
            }
            stage("Example test") {
              steps {
                example_test_linux("pytorch", "cpu", "Debug")
              }
            }
            stage("Tutorial test") {
              steps {
                tutorial_test_linux("pytorch", "Debug")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Torch CPU (Win64)") {
          agent { label "windows" }
          stages {
            stage("Unit test") {
              steps {
                unit_test_win64("pytorch", "cpu", "Release")
              }
            }
            stage("Example test") {
              steps {
                example_test_win64("pytorch", "cpu", "Release")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Torch CPU (Win64) (Debug)") {
          agent { label "windows" }
          stages {
            stage("Unit test") {
              steps {
                unit_test_win64("pytorch", "cpu", "Debug")
              }
            }
            stage("Example test") {
              steps {
                example_test_win64("pytorch", "cpu", "Debug")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Torch GPU") {
          agent {
            docker {
              label "linux-gpu-node"
              image "dgllib/dgl-ci-gpu:conda"
              args "--runtime nvidia"
              alwaysPull true
            }
          }
          stages {
            stage("Unit test") {
              steps {
                sh "nvidia-smi"
                unit_test_linux("pytorch", "gpu", "Release")
              }
            }
            stage("Example test") {
              steps {
                example_test_linux("pytorch", "gpu", "Release")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Torch GPU (Debug)") {
          agent {
            docker {
              label "linux-gpu-node"
              image "dgllib/dgl-ci-gpu:conda"
              args "--runtime nvidia"
              alwaysPull true
            }
          }
          stages {
            stage("Unit test") {
              steps {
                sh "nvidia-smi"
                unit_test_linux("pytorch", "gpu", "Debug")
              }
            }
            stage("Example test") {
              steps {
                example_test_linux("pytorch", "gpu", "Debug")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Torch GPU (Win64)") {
          agent { label "windows" }
          stages {
            stage("Unit test") {
              steps {
                unit_test_win64("pytorch", "gpu", "Release")
              }
            }
            stage("Example test") {
              steps {
                example_test_win64("pytorch", "gpu", "Release")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("Torch GPU (Win64) (Debug)") {
          agent { label "windows" }
          stages {
            stage("Unit test") {
              steps {
                unit_test_win64("pytorch", "gpu", "Debug")
              }
            }
            stage("Example test") {
              steps {
                example_test_win64("pytorch", "gpu", "Debug")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("MXNet CPU") {
          agent { 
            docker {
              label "linux-c52x-node"
              image "dgllib/dgl-ci-cpu:conda" 
              alwaysPull true
            }
          }
          stages {
            stage("Unit test") {
              steps {
                unit_test_linux("mxnet", "cpu", "Release")
              }
            }
            //stage("Tutorial test") {
            //  steps {
            //    tutorial_test_linux("mxnet")
            //  }
            //}
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
        stage("MXNet GPU") {
          agent {
            docker {
              label "linux-gpu-node" 
              image "dgllib/dgl-ci-gpu:conda"
              args "--runtime nvidia"
              alwaysPull true
            }
          }
          stages {
            stage("Unit test") {
              steps {
                sh "nvidia-smi"
                unit_test_linux("mxnet", "gpu", "Release")
              }
            }
          }
          post {
            always {
              cleanWs disableDeferredWipeout: true, deleteDirs: true
            }
          }
        }
      }
    }
  }
  post {
    always {
      node('windows') {
        bat "rmvirtualenv ${BUILD_TAG}"
      }
    }
  }
}
