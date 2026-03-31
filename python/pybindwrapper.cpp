#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/attr.h>
#include <pybind11/stl.h>
#include <pybind11/detail/common.h>
#include <pybind11/eigen.h>

#include "sparsepc/version.hpp"
#include "sparsepc/core.hpp"

PYBIND11_MODULE(sparsepc, mainmodule)
{
#ifdef SPARSEPC_VERSION
    mainmodule.attr("__version__") = SPARSEPC_MACRO_STRINGIFY(SPARSEPC_VERSION);
#else
    mainmodule.attr("__version__") = "dev";
#endif

    mainmodule.doc() = "sparsepc sparse modeling module.";

    using Index = sparsepc::Index;

    using ComponentState = sparsepc::ComponentState;
    pybind11::enum_<ComponentState>(mainmodule, "ComponentState", pybind11::arithmetic(), "State of a candidate component")
        .value("Validated", ComponentState::Validated, "Validated component")
        .value("Unvalidated", ComponentState::Unvalidated, "Still candidate component")
        .value("Unknown", ComponentState::Unknown, "Neither validated nor candidate")
        .export_values();

    using Vectord = sparsepc::Vector<double>;
    using Componentd = sparsepc::Component<double>;
    pybind11::class_<Componentd>(mainmodule, "Componentd")
        .def(pybind11::init<>())
        .def(pybind11::init<Index>(), pybind11::arg("n"))
        .def(pybind11::init<double, Vectord>(), 
            pybind11::arg("value"),
            pybind11::arg("vector"))
        .def_readwrite("state", &Componentd::state)
        .def_readwrite("value", &Componentd::value)
        .def_readwrite("vector", &Componentd::vector)
        .def_readwrite("q", &Componentd::q);

    mainmodule.def("toMatrixd", &sparsepc::toMatrix<double>,
        pybind11::arg("components"));

    using EigenSolverd = sparsepc::EigenSolver<double>;
    using EigenSolverParamd = EigenSolverd::Param;
    pybind11::class_<EigenSolverParamd>(mainmodule, "EigenSolverParamd")
        .def(pybind11::init<double, unsigned int>(),
            pybind11::arg("epsilon") = 1e-4,
            pybind11::arg("maximumNumberOfIterations") = 1000000U)
        .def_readonly("epsilon", &EigenSolverParamd::epsilon)
        .def_readonly("maximumNumberOfIterations", &EigenSolverParamd::maximumNumberOfIterations);

    pybind11::class_<EigenSolverd>(mainmodule, "EigenSolverd")
        .def(pybind11::init<EigenSolverParamd>(), pybind11::arg("param") = EigenSolverParamd{})
        .def("maximumValue", &EigenSolverd::maximumValue, pybind11::arg("sigma"))
        .def("maximumValueElement", &EigenSolverd::maximumValueElement, pybind11::arg("sigma"));

    using Vectorf = sparsepc::Vector<float>;
    using Componentf = sparsepc::Component<float>;
    pybind11::class_<Componentf>(mainmodule, "Componentf")
        .def(pybind11::init<>())
        .def(pybind11::init<Index>(), pybind11::arg("n"))
        .def(pybind11::init<float, Vectorf>(),
            pybind11::arg("value"),
            pybind11::arg("vector"))
        .def_readwrite("state", &Componentf::state)
        .def_readwrite("value", &Componentf::value)
        .def_readwrite("vector", &Componentf::vector)
        .def_readwrite("q", &Componentf::q);

    mainmodule.def("toMatrixf", &sparsepc::toMatrix<float>,
        pybind11::arg("components"));

    using EigenSolverf = sparsepc::EigenSolver<float>;
    using EigenSolverParamf = EigenSolverf::Param;
    pybind11::class_<EigenSolverParamf>(mainmodule, "EigenSolverParamf")
        .def(pybind11::init<float, unsigned int>(),
            pybind11::arg("epsilon") = 1e-4f,
            pybind11::arg("maximumNumberOfIterations") = 1000000U)
        .def_readonly("epsilon", &EigenSolverParamf::epsilon)
        .def_readonly("maximumNumberOfIterations", &EigenSolverParamf::maximumNumberOfIterations);

    pybind11::class_<EigenSolverf>(mainmodule, "EigenSolverf")
        .def(pybind11::init<EigenSolverParamf>(), pybind11::arg("param") = EigenSolverParamf{})
        .def("maximumValue", &EigenSolverf::maximumValue, pybind11::arg("sigma"))
        .def("maximumValueElement", &EigenSolverf::maximumValueElement, pybind11::arg("sigma"));

    auto m = mainmodule.def_submodule("linearmodel", "Linear model module.");

    using BackwardGspcad = sparsepc::linearmodel::BackwardGspca<double>;
    using BackwardGspcaParamd = BackwardGspcad::Param;
    using BackwardGspcaModelParamd = BackwardGspcad::ModelParam;
    pybind11::class_<BackwardGspcaModelParamd>(m, "BackwardGspcaModelParamd")
    .def(pybind11::init<Index, EigenSolverd, double>(),
        pybind11::arg("k") = static_cast<Index>(1),
        pybind11::arg("eigenSolver") = EigenSolverd{},
        pybind11::arg("zero") = 1e-6)
        .def_readonly("k", &BackwardGspcaModelParamd::k)
        .def_readonly("eigenSolver", &BackwardGspcaModelParamd::eigenSolver)
        .def_readonly("zero", &BackwardGspcaModelParamd::zero);

    pybind11::class_<BackwardGspcaParamd>(m, "BackwardGspcaParamd")
        .def(pybind11::init<std::vector<BackwardGspcaModelParamd>>(), 
            pybind11::arg("modelParams"))
        .def_readonly("modelParams", &BackwardGspcaParamd::modelParams)
        .def_readonly("nbComponents", &BackwardGspcaParamd::nbComponents);

    using Matrixd = sparsepc::Matrix<double>;
    pybind11::class_<BackwardGspcad>(m, "BackwardGspcad")
        .def(pybind11::init<BackwardGspcaParamd>(),
            pybind11::arg("param"))
        .def("run", &BackwardGspcad::run,
            pybind11::arg("sigma"))
        .def_static("computeNextComponentCandidates",
            &BackwardGspcad::computeNextComponentCandidates<Componentd>,
            pybind11::arg("sigma"), 
            pybind11::arg("param"),
            pybind11::arg("validatedComponents") = std::vector<Componentd>{},
            pybind11::arg("progressBar") = nullptr);

    using ForwardGspcad = sparsepc::linearmodel::ForwardGspca<double>;
    using ForwardGspcaParamd = ForwardGspcad::Param;
    using ForwardGspcaModelParamd = ForwardGspcad::ModelParam;
    pybind11::class_<ForwardGspcaModelParamd>(m, "ForwardGspcaModelParamd")
    .def(pybind11::init<Index, EigenSolverd, double>(),
        pybind11::arg("k") = static_cast<Index>(1),
        pybind11::arg("eigenSolver") = EigenSolverd{},
        pybind11::arg("zero") = 1e-6)
        .def_readonly("k", &ForwardGspcaModelParamd::k)
        .def_readonly("eigenSolver", &ForwardGspcaModelParamd::eigenSolver)
        .def_readonly("zero", &ForwardGspcaModelParamd::zero);

    pybind11::class_<ForwardGspcaParamd>(m, "ForwardGspcaParamd")
        .def(pybind11::init<std::vector<ForwardGspcaModelParamd>>(), 
            pybind11::arg("modelParams"))
        .def_readonly("modelParams", &ForwardGspcaParamd::modelParams)
        .def_readonly("nbComponents", &ForwardGspcaParamd::nbComponents);

    pybind11::class_<ForwardGspcad>(m, "ForwardGspcad")
        .def(pybind11::init<ForwardGspcaParamd>(),
            pybind11::arg("param"))
        .def("run", &ForwardGspcad::run,
            pybind11::arg("sigma"))
        .def_static("computeNextComponentCandidates",
            &ForwardGspcad::computeNextComponentCandidates<Componentd>,
            pybind11::arg("sigma"),
            pybind11::arg("param"),
            pybind11::arg("validatedComponents") = std::vector<Componentd>{},
            pybind11::arg("progressBar") = nullptr);

    using ParallelGspcad = sparsepc::linearmodel::ParallelGspca<double>;
    using ParallelGspcaParamd = ParallelGspcad::Param;
    using ParallelGspcaModelParamd = ParallelGspcad::ModelParam;
    pybind11::class_<ParallelGspcaModelParamd>(m, "ParallelGspcaModelParamd")
        .def(pybind11::init<Index, EigenSolverd, EigenSolverd, double>(),
            pybind11::arg("k") = static_cast<Index>(1),
            pybind11::arg("eigenSolverForForward") = EigenSolverd{},
            pybind11::arg("eigenSolverForBackward") = EigenSolverd{},
            pybind11::arg("zero") = 1e-6)
        .def_readonly("k", &ParallelGspcaModelParamd::k)
        .def_readonly("eigenSolverForForward", &ParallelGspcaModelParamd::eigenSolverForForward)
        .def_readonly("eigenSolverForBackward", &ParallelGspcaModelParamd::eigenSolverForBackward)
        .def_readonly("zero", &ParallelGspcaModelParamd::zero);

    pybind11::class_<ParallelGspcaParamd>(m, "ParallelGspcaParamd")
        .def(pybind11::init<std::vector<ParallelGspcaModelParamd>>(), 
            pybind11::arg("modelParams"))
        .def_readonly("modelParams", &ParallelGspcaParamd::modelParams)
        .def_readonly("nbComponents", &ParallelGspcaParamd::nbComponents);

    pybind11::class_<ParallelGspcad>(m, "ParallelGspcad")
        .def(pybind11::init<ParallelGspcaParamd>(),
            pybind11::arg("param"))
        .def("run", &ParallelGspcad::run,
            pybind11::arg("sigma"))
        .def_static("computeNextComponentCandidates",
            &ParallelGspcad::computeNextComponentCandidates<Componentd>,
            pybind11::arg("sigma"),
            pybind11::arg("param"),
            pybind11::arg("validatedComponents") = std::vector<Componentd>{},
            pybind11::arg("progressBar") = nullptr);

    using Dcad = sparsepc::linearmodel::Dca<double>;
    using DcaParamd = Dcad::Param;
    using DcaModelParamd = Dcad::ModelParam;
    pybind11::class_<DcaModelParamd>(m, "DcaModelParamd")
        .def(pybind11::init<Index, EigenSolverd, double, double, unsigned int, double>(),
            pybind11::arg("k") = static_cast<Index>(1),
            pybind11::arg("eigenSolver") = EigenSolverd{},
            pybind11::arg("t") = 1000000.0,
            pybind11::arg("tolerance") = 1e-4,
            pybind11::arg("maximumNumberOfIterations") = 10000U,
            pybind11::arg("zero") = 1e-6)
        .def_readonly("k", &DcaModelParamd::k)
        .def_readonly("eigenSolver", &DcaModelParamd::eigenSolver)
        .def_readonly("t", &DcaModelParamd::t)
        .def_readonly("tolerance", &DcaModelParamd::tolerance)
        .def_readonly("maximumNumberOfIterations", &DcaModelParamd::maximumNumberOfIterations)
        .def_readonly("zero", &DcaModelParamd::zero);

    pybind11::class_<DcaParamd>(m, "DcaParamd")
        .def(pybind11::init<std::vector<DcaModelParamd>>(), pybind11::arg("modelParams"))
        .def_readonly("modelParams", &DcaParamd::modelParams)
        .def_readonly("nbComponents", &DcaParamd::nbComponents);

    pybind11::class_<Dcad>(m, "Dcad")
        .def(pybind11::init<DcaParamd>(),
            pybind11::arg("param"))
        .def("run", &Dcad::run,
            pybind11::arg("sigma"))
        .def_static("computeNextComponentCandidates",
            &Dcad::computeNextComponentCandidates<Componentd>,
            pybind11::arg("sigma"),
            pybind11::arg("param"),
            pybind11::arg("validatedComponents") = std::vector<Componentd>{},
            pybind11::arg("progressBar") = nullptr);

    using BackwardGspcaf = sparsepc::linearmodel::BackwardGspca<float>;
    using BackwardGspcaParamf = BackwardGspcaf::Param;
    using BackwardGspcaModelParamf = BackwardGspcaf::ModelParam;
    pybind11::class_<BackwardGspcaModelParamf>(m, "BackwardGspcaModelParamf")
        .def(pybind11::init<Index, EigenSolverf, float>(),
            pybind11::arg("k") = static_cast<Index>(1),
            pybind11::arg("eigenSolver") = EigenSolverf{},
            pybind11::arg("zero") = 1e-6f)
        .def_readonly("k", &BackwardGspcaModelParamf::k)
        .def_readonly("eigenSolver", &BackwardGspcaModelParamf::eigenSolver)
        .def_readonly("zero", &BackwardGspcaModelParamf::zero);

    pybind11::class_<BackwardGspcaParamf>(m, "BackwardGspcaParamf")
        .def(pybind11::init<std::vector<BackwardGspcaModelParamf>>(),
            pybind11::arg("modelParams"))
        .def_readonly("modelParams", &BackwardGspcaParamf::modelParams)
        .def_readonly("nbComponents", &BackwardGspcaParamf::nbComponents);

    using Matrixf = sparsepc::Matrix<float>;
    pybind11::class_<BackwardGspcaf>(m, "BackwardGspcaf")
        .def(pybind11::init<BackwardGspcaParamf>(),
            pybind11::arg("param"))
        .def("run", &BackwardGspcaf::run,
            pybind11::arg("sigma"))
        .def_static("computeNextComponentCandidates",
            &BackwardGspcaf::computeNextComponentCandidates<Componentf>,
            pybind11::arg("sigma"),
            pybind11::arg("param"),
            pybind11::arg("validatedComponents") = std::vector<Componentf>{},
            pybind11::arg("progressBar") = nullptr);

    using ForwardGspcaf = sparsepc::linearmodel::ForwardGspca<float>;
    using ForwardGspcaParamf = ForwardGspcaf::Param;
    using ForwardGspcaModelParamf = ForwardGspcaf::ModelParam;
    pybind11::class_<ForwardGspcaModelParamf>(m, "ForwardGspcaModelParamf")
        .def(pybind11::init<Index, EigenSolverf, float>(),
            pybind11::arg("k") = static_cast<Index>(1),
            pybind11::arg("eigenSolver") = EigenSolverf{},
            pybind11::arg("zero") = 1e-6f)
        .def_readonly("k", &ForwardGspcaModelParamf::k)
        .def_readonly("eigenSolver", &ForwardGspcaModelParamf::eigenSolver)
        .def_readonly("zero", &ForwardGspcaModelParamf::zero);

    pybind11::class_<ForwardGspcaParamf>(m, "ForwardGspcaParamf")
        .def(pybind11::init<std::vector<ForwardGspcaModelParamf>>(),
            pybind11::arg("modelParams"))
        .def_readonly("modelParams", &ForwardGspcaParamf::modelParams)
        .def_readonly("nbComponents", &ForwardGspcaParamf::nbComponents);

    pybind11::class_<ForwardGspcaf>(m, "ForwardGspcaf")
        .def(pybind11::init<ForwardGspcaParamf>(),
            pybind11::arg("param"))
        .def("run", &ForwardGspcaf::run,
            pybind11::arg("sigma"))
        .def_static("computeNextComponentCandidates",
            &ForwardGspcaf::computeNextComponentCandidates<Componentf>,
            pybind11::arg("sigma"),
            pybind11::arg("param"),
            pybind11::arg("validatedComponents") = std::vector<Componentf>{},
            pybind11::arg("progressBar") = nullptr);

    using ParallelGspcaf = sparsepc::linearmodel::ParallelGspca<float>;
    using ParallelGspcaParamf = ParallelGspcaf::Param;
    using ParallelGspcaModelParamf = ParallelGspcaf::ModelParam;
    pybind11::class_<ParallelGspcaModelParamf>(m, "ParallelGspcaModelParamf")
        .def(pybind11::init<Index, EigenSolverf, EigenSolverf, float>(),
            pybind11::arg("k") = static_cast<Index>(1),
            pybind11::arg("eigenSolverForForward") = EigenSolverf{},
            pybind11::arg("eigenSolverForBackward") = EigenSolverf{},
            pybind11::arg("zero") = 1e-6f)
        .def_readonly("k", &ParallelGspcaModelParamf::k)
        .def_readonly("eigenSolverForForward", &ParallelGspcaModelParamf::eigenSolverForForward)
        .def_readonly("eigenSolverForBackward", &ParallelGspcaModelParamf::eigenSolverForBackward)
        .def_readonly("zero", &ParallelGspcaModelParamf::zero);

    pybind11::class_<ParallelGspcaParamf>(m, "ParallelGspcaParamf")
        .def(pybind11::init<std::vector<ParallelGspcaModelParamf>>(),
            pybind11::arg("modelParams"))
        .def_readonly("modelParams", &ParallelGspcaParamf::modelParams)
        .def_readonly("nbComponents", &ParallelGspcaParamf::nbComponents);

    pybind11::class_<ParallelGspcaf>(m, "ParallelGspcaf")
        .def(pybind11::init<ParallelGspcaParamf>(),
            pybind11::arg("param"))
        .def("run", &ParallelGspcaf::run,
            pybind11::arg("sigma"))
        .def_static("computeNextComponentCandidates",
            &ParallelGspcaf::computeNextComponentCandidates<Componentf>,
            pybind11::arg("sigma"),
            pybind11::arg("param"),
            pybind11::arg("validatedComponents") = std::vector<Componentf>{},
            pybind11::arg("progressBar") = nullptr);

    using Dcaf = sparsepc::linearmodel::Dca<float>;
    using DcaParamf = Dcaf::Param;
    using DcaModelParamf = Dcaf::ModelParam;
    pybind11::class_<DcaModelParamf>(m, "DcaModelParamf")
        .def(pybind11::init<Index, EigenSolverf, float, float, unsigned int, float>(),
            pybind11::arg("k") = static_cast<Index>(1),
            pybind11::arg("eigenSolver") = EigenSolverf{},
            pybind11::arg("t") = 1000000.0f,
            pybind11::arg("tolerance") = 1e-4f,
            pybind11::arg("maximumNumberOfIterations") = 10000U,
            pybind11::arg("zero") = 1e-6f)
        .def_readonly("k", &DcaModelParamf::k)
        .def_readonly("eigenSolver", &DcaModelParamf::eigenSolver)
        .def_readonly("t", &DcaModelParamf::t)
        .def_readonly("tolerance", &DcaModelParamf::tolerance)
        .def_readonly("maximumNumberOfIterations", &DcaModelParamf::maximumNumberOfIterations)
        .def_readonly("zero", &DcaModelParamf::zero);

    pybind11::class_<DcaParamf>(m, "DcaParamf")
        .def(pybind11::init<std::vector<DcaModelParamf>>(), pybind11::arg("modelParams"))
        .def_readonly("modelParams", &DcaParamf::modelParams)
        .def_readonly("nbComponents", &DcaParamf::nbComponents);

    pybind11::class_<Dcaf>(m, "Dcaf")
        .def(pybind11::init<DcaParamf>(),
            pybind11::arg("param"))
        .def("run", &Dcaf::run,
            pybind11::arg("sigma"))
        .def_static("computeNextComponentCandidates",
            &Dcaf::computeNextComponentCandidates<Componentf>,
            pybind11::arg("sigma"),
            pybind11::arg("param"),
            pybind11::arg("validatedComponents") = std::vector<Componentf>{},
            pybind11::arg("progressBar") = nullptr);
}
