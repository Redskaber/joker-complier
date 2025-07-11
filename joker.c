/*
		This is the bytecode virtual machine.
		It will execute bytecode generated by the compiler.
		The bytecode will be stored in a file.
		The virtual machine will read the bytecode from the file and execute it.

		GC， 垃圾回收机制， 自动释放不再使用的内存，减少内存碎片。
		LCL(life cycle label) 局部变量表，存放函数调用时临时变量的生命周期。

		TODO:
			base Heap-Stack-Machine (slow and easy)
			base Register-Machine	(quick and dirty)
*/
#include "common.h"
#include "repl.h"

#if enable_depart_debug
#include "../tests/dpart/test_pool.h"
int main(int argc, char* argv[]) {
    (void)argc, (void)argv;

    test_basic_operation();
    test_edge_cases();
    test_memory_leak();

    benchmark_single_thread();
    benchmark_batch_ops();

    // 新增测试
    test_object_lifecycle();
    test_statistics();
    test_alignment();

    return 0;
}
#else
int main(int argc, char* argv[]) {
    (void)argc, (void)argv;
    return joker_entry(argc, argv);
}
#endif
