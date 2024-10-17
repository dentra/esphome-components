# Declaration

Allows you to declare variables, functions, classes, etc. directly in yaml instead of a separate header file.

Parameters:

- namespace: Optional string or list of strings. Namespace to wrap lambda code.
- internal: boolean. Wrap or not to private anonymous namespace. Defaults: true.
- lambda: c++ code to be included in declaration.
- include: one or list of includes.

## Configuration

```yaml
external_components:
  - source: github://dentra/esphome-components
    components: [declaration]

declaration:
  # Declaring private variable in anonymous namespace.
  # Generated code:
  # namespace {
  # int some_var = 0;
  # }
  - lambda: |-
      int some_var = 0;

  # Declaring private enum in separate namespace "ns1".
  # Generated code:
  # namespace {
  # namespace {
  # enum SomeEnum { ONE, TWO, THREE };
  # } // namespace ns1
  # } // namespace /* anonymous */
  - namespace: ns1
    lambda: |-
      enum SomeEnum { ONE, TWO, THREE };

  # Declaring public struct in separate namespace "ns2" but without anonymous namespace wrap, it might be used outside main.cpp.
  # Generated code:
  # namespace ns2 {
  # struct SomeStruct { bool value; };
  # } // namespace ns2
  - namespace: ns2
    internal: false
    lambda: |-
      struct SomeStruct { bool value; };
```
