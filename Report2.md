任务一
在父类ColumnVector中增加add虚函数定义，并在子类DateColumnVector、DecimalColum
nVector、TimestampColumnVector 中实现 add 方法和 ensureSize 辅助方法。

[LongColumnVector.cpp](./pixels-core/lib/vector/LongColumnVector.cpp)
[DateColumnVector.cpp](./pixels-core/lib/vector/DateColumnVector.cpp)
[DecimalColumnVector.cpp](./pixels-core/lib/vector/DecimalColumnVector.cpp)
[TimestampColumnVector.cpp](./pixels-core/lib/vector/TimestampColumnVector.cpp)


任务二
实现上述几种类型的ColumnWriter,可以成功写入.tbl 文件到.pxl 文件中。以上类的头文件
在源代码的cpp/pixels-core/include/writer/ 目录下，源文件在源代码的cpp/pixels-core/lib/writer/ 目
录下。


 任务三
实现写入文件可以被duckdb通过pixels正确读取，相关的测试文件可以参考pixels-duckdb
 /examples/pixels-example, 也可以直接运行 duckdb 进行测试, 因为编译 duckbd 时，已经将 pixels
extension 链接了进去