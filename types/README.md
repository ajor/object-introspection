
## New options
`measure_template_params`
The indexes of the template parameters to pay attention to for this container.
If this is not set, use all a container's template parameters.

`stub_template_params`
The indexes of template parameters to be replaced by Dummy objects of the same size.

All other template parameters will be left as-in and not measured.

## Old options
`numTemplateParams`
The first `numTemplateParams` template parameters for this container represent the types for the data we want to process.
If this is not set, use all a container's template parameters.
All of a container's parameters will still be enumerated and output in CodeGen, regardless of this setting.

`underlyingContainerIndex`
Only pay attention to this single template parameter for a container adapter.
Must not be set at the same time as `numTemplateParams`.

`allocatorIndex`
Index of a template parameter representing an allocator. It will be not be used when CodeGenning this container.

`replaceTemplateParamIndex`
Indexes of template parameters to be stubbed out, i.e. replaced with dummy structs of a given size.
Used for custom hashers and comparers.




Container params:

Options:
- measure - contained types or underlying containers
- keep - e.g. contained type for a container adapter
- stub - e.g. comparitors, hashers
- remove - must be at end. can these be stubbed instead?

Must have a way of saying which category the tail of the parameter list falls under. (there may be a variable number of params)

New config:
- by default measure all parameters
  - The tail parameters in a variadic template will be measured
- `stub_template_params`
- `underlying_container_index`
