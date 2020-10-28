#include <mitsuba/render/emitter.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/python/python.h>

/// Trampoline for derived types implemented in Python
MTS_VARIANT class PyEmitter : public Emitter<Float, Spectrum> {
public:
    MTS_IMPORT_TYPES(Emitter)

    PyEmitter(const Properties &props) : Emitter(props) { }

    std::pair<Ray3f, Spectrum>
    sample_ray(Float time, Float sample1, const Point2f &sample2,
           const Point2f &sample3, Mask active) const override {
        using Return = std::pair<Ray3f, Spectrum>;
        PYBIND11_OVERRIDE_PURE(Return, Emitter, sample_ray, time, sample1, sample2, sample3,
                               active);
    }

    std::pair<DirectionSample3f, Spectrum>
    sample_direction(const Interaction3f &ref,
                     const Point2f &sample,
                     Mask active) const override {
        using Return = std::pair<DirectionSample3f, Spectrum>;
        PYBIND11_OVERRIDE_PURE(Return, Emitter, sample_direction, ref, sample, active);
    }

    Float pdf_direction(const Interaction3f &ref,
                        const DirectionSample3f &ds,
                        Mask active) const override {
        PYBIND11_OVERRIDE_PURE(Float, Emitter, pdf_direction, ref, ds, active);
    }

    Spectrum eval(const SurfaceInteraction3f &si, Mask active) const override {
        PYBIND11_OVERRIDE_PURE(Spectrum, Emitter, eval, si, active);
    }

    ScalarBoundingBox3f bbox() const override {
        PYBIND11_OVERRIDE_PURE(ScalarBoundingBox3f, Emitter, bbox,);
    }


    std::string to_string() const override {
        PYBIND11_OVERRIDE_PURE(std::string, Emitter, to_string,);
    }
};

MTS_PY_EXPORT(Emitter) {
    MTS_PY_IMPORT_TYPES()
    using PyEmitter = PyEmitter<Float, Spectrum>;

    auto emitter = py::class_<Emitter, PyEmitter, Endpoint, ref<Emitter>>(m, "Emitter", D(Emitter))
        .def(py::init<const Properties&>())
        .def_method(Emitter, is_environment)
        .def_method(Emitter, flags);

    if constexpr (ek::is_array_v<EmitterPtr>) {
        py::object ek       = py::module_::import("enoki"),
                   ek_array = ek.attr("ArrayBase");

        py::class_<EmitterPtr> cls(m, "EmitterPtr", ek_array);

        cls.def("sample_ray",
                [](EmitterPtr ptr, Float time, Float sample1, const Point2f &sample2,
                const Point2f &sample3, Mask active) {
                    return ptr->sample_ray(time, sample1, sample2, sample3, active);
                },
                "time"_a, "sample1"_a, "sample2"_a, "sample3"_a, "active"_a = true,
                D(Endpoint, sample_ray))
        .def("sample_direction",
                [](EmitterPtr ptr, const Interaction3f &it, const Point2f &sample, Mask active) {
                return ptr->sample_direction(it, sample, active);
                },
                "it"_a, "sample"_a, "active"_a = true,
                D(Endpoint, sample_direction))
        .def("pdf_direction",
                [](EmitterPtr ptr, const Interaction3f &it, const DirectionSample3f &ds, Mask active) {
                    return ptr->pdf_direction(it, ds, active);
                },
                "it"_a, "ds"_a, "active"_a = true,
                D(Endpoint, pdf_direction))
        .def("eval",
                [](EmitterPtr ptr, const SurfaceInteraction3f &si, Mask active) {
                    return ptr->eval(si, active);
                },
                "si"_a, "active"_a = true, D(Endpoint, eval))
        .def("flags", [](EmitterPtr ptr) { return ptr->flags(); }, D(Emitter, flags))
        .def("is_environment",
             [](EmitterPtr ptr) { return ptr->is_environment(); },
             D(Emitter, is_environment));

        bind_enoki_ptr_array(cls);

        pybind11_type_alias<UInt32, ek::replace_scalar_t<UInt32, EmitterFlags>>();
    }

    MTS_PY_REGISTER_OBJECT("register_emitter", Emitter)
}
