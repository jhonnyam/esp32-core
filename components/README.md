# Components

"Component" is a term used by the espressif idf framework to separate code
into libraries and define dependencies.

## Adding a new component
A basic component in this project has following structure:
```
basic_component
├── include
│   ├── header1.h
│   ├── header2.h
│   └── ...
├── CMakeLists.txt
├── code1.c
├── code2.c
├── README.md
└── ...
```

- `/basic_component` is the name of the component
  - **ATTENTION**: a component will replace any other component with the same name
- `/CMakeLists.txt` defines how the component should be build and what to export
- `/README.md` should give a additional information about the component

### Simple CMakeLists.txt
A simple variant of the CMakeLists.txt only consists of the module registration:
```
idf_component_register(
  SRCS
    code1.c
    code2.c
    ...
  INCLUDE_DIRS
    include
  REQUIRES
    other_component
)
```

Check out the
[idf documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html)
for further information