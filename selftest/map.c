#include <linux/module.h>
#include "ktest.h"
#include "ktest_map.h"
#include "ktest_syms.h"

MODULE_LICENSE("GPL");

struct map_test_ctx {
	struct ktest_context k;
};

struct map_test_ctx s_mctx[3];

/* Declare a simple handle with no contexts for simple (unparameterized) tests: */
DECLARE_DEFAULT_HANDLE();

/* For tests that defines multiple test cases
 * (e.g. if the test scope requires application of each test on several devices or
 *  other abstract contexts, definable by the test module)
 */
DECLARE_KTEST_HANDLE(dual_handle);
DECLARE_KTEST_HANDLE(single_handle);
DECLARE_KTEST_HANDLE(no_handle);

struct map_test_ctx *to_mctx(struct ktest_context *ctx)
{
	return container_of(ctx, struct map_test_ctx, k);
}


TEST(any, simplemap)
{
	int i;
	const int nelems = 3;
	struct map_test_ctx *mctx = to_mctx(ctx);

	struct ktest_map tm;
	struct myelem {
		struct ktest_map_elem foo;
	};

	struct myelem e[nelems];

	if (mctx) {
		tlog(T_DEBUG, "ctx %s\n", mctx->k.elem.name);
	} else
		tlog(T_DEBUG, "ctx <none>\n");

	ktest_map_init(&tm);
	EXPECT_INT_EQ(0, ktest_map_elem_init(&e[0].foo, "foo"));
	EXPECT_INT_EQ(0, ktest_map_elem_init(&e[1].foo, "bar"));
	EXPECT_INT_EQ(0, ktest_map_elem_init(&e[2].foo, "zax"));

	for (i = 0; i < nelems; i++) {
		EXPECT_LONG_EQ(i, ktest_map_size(&tm));
		EXPECT_INT_EQ(0, ktest_map_insert(&tm, &e[i].foo));
	}
	EXPECT_LONG_EQ(i, ktest_map_size(&tm));

	/* Should be sorted alphabetically so we get 'bar' back: */
	EXPECT_ADDR_EQ(&e[1].foo, ktest_map_find_first(&tm));

	for (i = 0; i < nelems; i++) {
		EXPECT_LONG_EQ(nelems - i, ktest_map_size(&tm));
		EXPECT_ADDR_EQ(&e[i].foo, ktest_map_remove(&tm, e[i].foo.name));
	}
	EXPECT_LONG_EQ(0, ktest_map_size(&tm));
}

TEST(any, dummy)
{
	EXPECT_TRUE(true);
}


static void add_map_tests(void)
{
	ADD_TEST(dummy);
	ADD_TEST_TO(dual_handle, simplemap);
}


static int __init maptest_init(void)
{
	int ret = ktest_context_add(&dual_handle, &s_mctx[0].k, "map1");
	if (ret)
		return ret;

	ret = ktest_context_add(&dual_handle, &s_mctx[1].k, "map2");
	if (ret)
		return ret;

	ret = ktest_context_add(&single_handle, &s_mctx[2].k, "map3");
	if (ret)
		return ret;

	resolve_symbols();

	add_map_tests();
	tlog(T_INFO, "maptest: loaded\n");
	return 0;
}

static void __exit maptest_exit(void)
{
	KTEST_HANDLE_CLEANUP(single_handle);
	KTEST_HANDLE_CLEANUP(dual_handle);
	KTEST_HANDLE_CLEANUP(no_handle);
	KTEST_CLEANUP();
	tlog(T_INFO, "map: unloaded\n");
	/* Nothing to do here */
}


module_init(maptest_init);
module_exit(maptest_exit);