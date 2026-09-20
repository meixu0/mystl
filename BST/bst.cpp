#include "bst.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

// Linux/POSIX 测试程序。每个用例在子进程运行，避免头文件中的崩溃
// 中断其他测试；超时、异常、信号退出均计为失败。
// 编译运行：g++ -std=c++11 -Wall -Wextra -pedantic bst.cpp -o /tmp/bst_test
//           /tmp/bst_test
// 无法实例化的接口可分别检查（N 为 1 到 5）：
// g++ -std=c++11 -DBST_COMPILE_CHECK=N -fsyntax-only bst.cpp
// 1: ++it，2: it++，3: --it，4: it--，5: erase

namespace {
using Tree = bst::BST<int>;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void fill(Tree& tree, std::initializer_list<int> values) {
    for (int value : values) tree.insert(value);
}

struct Test {
    const char* name;
    void (*run)();
};

const Test tests[] = {
    {"默认构造：size/empty/end", [] {
        Tree tree;
        check(tree.size() == 0 && tree.empty(), "空树状态错误");
        check(tree.end() == tree.end(), "end 比较错误");
    }},
    {"插入、重复值与 size", [] {
        Tree tree;
        fill(tree, {8, 3, 10, 1, 6, 14, 4, 7, 13, 6});
        check(tree.size() == 10 && !tree.empty(), "插入后元素计数错误");
    }},
    {"find 查找已有值（含 const 重载）", [] {
        Tree tree;
        fill(tree, {8, 3, 10, 1, 6, 14, 4, 7, 13});
        const Tree& view = tree;
        for (int value : {8, 3, 10, 1, 6, 14, 4, 7, 13}) {
            auto it = tree.find(value);
            check(it != tree.end() && *it == value, "find 查找已有值失败");
            auto cit = view.find(value);
            check(cit != tree.end() && *cit == value, "const find 失败");
        }
    }},
    {"find 查找不存在的值", [] {
        Tree tree;
        fill(tree, {8, 3, 10});
        check(tree.find(2) == tree.end(), "查找失败应返回 end");
    }},
    {"const find 查找不存在的值", [] {
        Tree tree;
        fill(tree, {8, 3, 10});
        const Tree& view = tree;
        check(view.find(20) == tree.end(), "const 查找失败应返回 end");
    }},
    {"空树 find", [] {
        Tree tree;
        check(tree.find(1) == tree.end(), "空树 find 应返回 end");
    }},
    {"空树 const find", [] {
        Tree tree;
        const Tree& view = tree;
        check(view.find(1) == tree.end(), "空树 const find 应返回 end");
    }},
    {"空树 begin/end", [] {
        Tree tree;
        check(tree.begin() == tree.end(), "空树 begin 应等于 end");
    }},
    {"begin 指向最小元素", [] {
        Tree tree;
        fill(tree, {8, 3, 10, 1, 6, 9});
        check(*tree.begin() == 1, "begin 未指向最小元素");
    }},
    {"单节点迭代器比较与解引用", [] {
        Tree tree;
        fill(tree, {5});
        check(tree.begin() != tree.end(), "非空树 begin 等于 end");
        check(tree.begin() == tree.find(5), "相同节点的迭代器不相等");
        *tree.begin() = 6;
        check(*tree.begin() == 6, "解引用应支持修改元素");
    }},
    {"successor：右子树、祖先和末尾", [] {
        // 节点类型公开，直接建立栈上节点以独立验证 successor。
        bst::TreeNode<int> root{4, nullptr, nullptr, nullptr};
        bst::TreeNode<int> left{2, nullptr, nullptr, &root};
        bst::TreeNode<int> right{6, nullptr, nullptr, &root};
        bst::TreeNode<int> middle{5, nullptr, nullptr, &right};
        root.lchild = &left;
        root.rchild = &right;
        right.lchild = &middle;
        Tree tree;
        check(tree.successor(&root) == &middle, "右子树最小节点错误");
        check(tree.successor(&left) == &root, "祖先后继错误");
        check(tree.successor(&middle) == &right, "直接父节点后继错误");
        check(tree.successor(&right) == nullptr, "最大值后继应为空");
    }},
    {"空树拷贝构造", [] {
        Tree source;
        Tree copy(source);
        check(copy.empty() && copy.size() == 0, "空树拷贝错误");
    }},
    {"拷贝构造及独立存储", [] {
        Tree source;
        fill(source, {8, 3, 10});
        Tree copy(source);
        check(copy.size() == source.size(), "拷贝大小错误");
        *copy.find(3) = 2;
        check(*source.find(3) == 3, "拷贝共享了节点");
        fill(copy, {12});
        check(source.size() == 3 && copy.size() == 4, "拷贝后插入影响原树");
    }},
    {"拷贝赋值覆盖已有内容", [] {
        Tree source, target;
        fill(source, {8, 3, 10});
        fill(target, {20, 15});
        target = source;
        check(target.size() == 3, "拷贝赋值未正确更新 size");
        check(*target.find(3) == 3, "拷贝赋值内容错误");
    }},
    {"空树拷贝赋值", [] {
        Tree source, target;
        fill(target, {1});
        target = source;
        check(target.empty(), "赋值空树后目标应为空");
    }},
    {"自拷贝赋值", [] {
        Tree tree;
        fill(tree, {8, 3, 10});
        tree = tree;
        check(tree.size() == 3 && *tree.find(8) == 8, "自赋值损坏树");
    }},
    {"移动构造及析构", [] {
        Tree source;
        fill(source, {8, 3, 10});
        Tree target(std::move(source));
        check(target.size() == 3 && *target.find(3) == 3, "移动构造内容错误");
        // 两个对象自然析构，检查是否出现重复释放。
    }},
    {"移动赋值及析构", [] {
        Tree source, target;
        fill(source, {8, 3, 10});
        fill(target, {20});
        target = std::move(source);
        check(target.size() == 3 && *target.find(3) == 3, "移动赋值内容错误");
    }},
    {"空树 clear 可重复调用", [] {
        Tree tree;
        tree.clear();
        tree.clear();
        check(tree.empty(), "空树 clear 状态错误");
    }},
    {"非空树 clear 状态及析构", [] {
        Tree tree;
        fill(tree, {8, 3, 10});
        tree.clear();
        check(tree.empty() && tree.size() == 0, "clear 未重置大小");
    }},
    {"clear 后重新插入", [] {
        Tree tree;
        fill(tree, {8, 3, 10});
        tree.clear();
        fill(tree, {5});
        check(tree.size() == 1 && *tree.find(5) == 5, "clear 后复用失败");
    }},
    {"字符串模板实例", [] {
        bst::BST<std::string> tree;
        for (std::string value : {"pear", "apple", "orange"}) tree.insert(value);
        check(tree.size() == 3, "字符串树大小错误");
        check(*tree.find("apple") == "apple", "字符串查找错误");
    }},
};
} // namespace

#ifdef BST_COMPILE_CHECK
void compile_check() {
    Tree tree;
    fill(tree, {8, 3, 10});
    auto it = tree.begin();
#if BST_COMPILE_CHECK == 1
    ++it;
#elif BST_COMPILE_CHECK == 2
    it++;
#elif BST_COMPILE_CHECK == 3
    --it;
#elif BST_COMPILE_CHECK == 4
    it--;
#elif BST_COMPILE_CHECK == 5
    tree.erase(3);
#else
#error BST_COMPILE_CHECK must be between 1 and 5
#endif
}
#endif

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    unsigned passed = 0;
    const unsigned total = sizeof(tests) / sizeof(tests[0]);
    for (const auto& test : tests) {
        std::printf("[RUN ] %s\n", test.name);
        const pid_t pid = fork();
        if (pid < 0) {
            std::perror("fork");
            return EXIT_FAILURE;
        }
        if (pid == 0) {
            const rlimit limit{0, 0};
            setrlimit(RLIMIT_CORE, &limit);
            alarm(3);
            try {
                test.run();
                std::exit(EXIT_SUCCESS);
            } catch (const std::exception& error) {
                std::fprintf(stderr, "  %s\n", error.what());
            } catch (...) {
                std::fprintf(stderr, "  未知异常\n");
            }
            std::exit(EXIT_FAILURE);
        }
        int status = 0;
        pid_t waited;
        do {
            waited = waitpid(pid, &status, 0);
        } while (waited < 0 && errno == EINTR);
        if (waited < 0) {
            std::perror("waitpid");
            return EXIT_FAILURE;
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            ++passed;
            std::printf("[PASS] %s\n", test.name);
        } else if (WIFSIGNALED(status)) {
            std::printf("[FAIL] %s（信号 %d）\n", test.name, WTERMSIG(status));
        } else {
            std::printf("[FAIL] %s（退出码 %d）\n", test.name, WEXITSTATUS(status));
        }
    }
    std::printf("\n通过 %u/%u，失败 %u。迭代器增减与 erase 请另运行编译检查。\n",
                passed, total, total - passed);
    return passed == total ? EXIT_SUCCESS : EXIT_FAILURE;
}
