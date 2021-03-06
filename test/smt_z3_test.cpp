#include "gtest/gtest.h"

#include "smt.h"
#include "smt_z3.h"

#include <sstream>
#include <cstdint>

using namespace smt;

TEST(SmtZ3Test, BvNoCastLiteralExpr)
{
  Z3Solver s;

  const LiteralExpr<Bv<int>> e0(42);
  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr expr(s.expr());
  EXPECT_TRUE(expr.is_bv());
  EXPECT_TRUE(expr.is_const());
  EXPECT_EQ(sizeof(int) * 8, expr.get_sort().bv_size());

  z3::solver& solver = s.solver();

  solver.add(42 == expr);
  EXPECT_EQ(z3::sat, solver.check());

  solver.add(42 != expr);
  EXPECT_EQ(z3::unsat, solver.check());
}

TEST(SmtZ3Test, BvCastLiteralExpr)
{
  Z3Solver s;

  const LiteralExpr<Bv<char>> e0('A');
  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr expr(s.expr());
  EXPECT_TRUE(expr.is_bv());
  EXPECT_TRUE(expr.is_const());
  EXPECT_EQ(sizeof(char) * 8, expr.get_sort().bv_size());

  z3::solver& solver = s.solver();

  solver.push();
  {
    solver.add('A' == expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add('A' != expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, Bv64CastLiteralExpr)
{
  Z3Solver s;

  const LiteralExpr<Bv<long>> e0(42L);
  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr expr(s.expr());
  EXPECT_TRUE(expr.is_bv());
  EXPECT_TRUE(expr.is_const());

  // due to Z3_mk_int64
  EXPECT_EQ(sizeof(int64_t) * 8, expr.get_sort().bv_size());

  z3::solver& solver = s.solver();

  solver.add(42 == expr);
  EXPECT_EQ(z3::sat, solver.check());

  solver.add(42 != expr);
  EXPECT_EQ(z3::unsat, solver.check());
}

TEST(SmtZ3Test, BoolLiteralExpr)
{
  Z3Solver s;

  const LiteralExpr<Bool, bool> e0(true);
  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr expr(s.expr());
  EXPECT_TRUE(expr.is_bool());
  EXPECT_TRUE(expr.is_const());

  z3::solver& solver = s.solver();

  solver.push();
  {
    solver.add(expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(!expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, IntLiteralExpr)
{
  Z3Solver s;

  const LiteralExpr<Int, char> e0('A');
  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr expr(s.expr());
  EXPECT_TRUE(expr.is_int());
  EXPECT_TRUE(expr.is_const());

  z3::solver& solver = s.solver();

  solver.push();
  {
    solver.add('A' == expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add('A' != expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, RealLiteralExpr)
{
  Z3Solver s;

  // note that float and double are unsupported
  const LiteralExpr<Real, int> e0(7);
  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr expr(s.expr());
  EXPECT_TRUE(expr.is_real());
  EXPECT_TRUE(expr.is_const());

  z3::solver& solver = s.solver();

  solver.push();
  {
    solver.add(7 == expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(7 != expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, Decl)
{
  Z3Solver s;
  constexpr size_t bv_size = sizeof(long) * 8;

  const Decl<Bv<long>> d0("x");
  Bv<long> e0_term = constant(d0);
  EXPECT_EQ(OK, static_cast<UnsafeTerm>(e0_term).encode(s));

  const z3::expr expr(s.expr());
  EXPECT_TRUE(expr.is_bv());
  EXPECT_TRUE(expr.is_const());
  EXPECT_EQ(bv_size, expr.get_sort().bv_size());

  z3::solver& solver = s.solver();

  solver.push();
  {
    solver.add(42 == expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(42 != expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    const z3::sort z3_sort(s.context().bv_sort(bv_size));
    const z3::expr z3_expr(s.context().constant("x", z3_sort));
    solver.add(z3_expr != expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, UnaryExpr)
{
  Z3Solver s;

  const Bv<int> e0_term(new LiteralExpr<Bv<int>>(42));
  const UnaryExpr<SUB, Bv<int>> e1(e0_term);

  EXPECT_EQ(OK, e1.encode(s));

  const z3::expr expr(s.expr());
  EXPECT_TRUE(expr.is_bv());
  EXPECT_TRUE(expr.is_app());
  EXPECT_EQ(sizeof(int) * 8, expr.get_sort().bv_size());

  z3::solver& solver = s.solver();

  solver.push();
  {
    solver.add(-42 == expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(-42 != expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, BinaryExpr)
{
  Z3Solver s;
  z3::expr expr(s.context());

  const Bv<long> e0_term(new LiteralExpr<Bv<long>>(42L));
  const Bv<long> e1_term(new LiteralExpr<Bv<long>>(7L));
  const BinaryExpr<ADD, Bv<long>> e2(e0_term, e1_term);

  EXPECT_EQ(OK, e2.encode(s));

  expr = s.expr();
  EXPECT_TRUE(expr.is_bv());
  EXPECT_TRUE(expr.is_app());
  EXPECT_EQ(sizeof(long) * 8, expr.get_sort().bv_size());

  z3::solver& solver = s.solver();

  solver.push();
  {
    solver.add(49 == expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(49 != expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();

  const BinaryExpr<LSS, Bv<long>, Bool> e3(e0_term, e1_term);

  EXPECT_EQ(OK, e3.encode(s));

  expr = s.expr();
  EXPECT_TRUE(expr.is_bool());
  EXPECT_TRUE(expr.is_app());

  solver.push();
  {
    solver.add(expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();

  const BinaryExpr<GTR, Bv<long>, Bool> e4(e0_term, e1_term);

  EXPECT_EQ(OK, e4.encode(s));

  expr = s.expr();
  EXPECT_TRUE(expr.is_bool());
  EXPECT_TRUE(expr.is_app());

  solver.push();
  {
    solver.add(expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, Distinct)
{
  Z3Solver s;

  const Bv<long> x = any<Bv<long>>("x");
  const Bv<long> y = any<Bv<long>>("y");
  const Bv<long> z = any<Bv<long>>("z");
  const Bv<long> w = any<Bv<long>>("w");

  Terms<Bv<long>> operand_terms(3);
  operand_terms.push_back(x);
  operand_terms.push_back(y);
  operand_terms.push_back(z);

  Bool d(distinct(std::move(operand_terms)));

  EXPECT_EQ(OK, static_cast<UnsafeTerm>(d).encode(s));
  std::stringstream out;
  out << s.expr();
  EXPECT_EQ("(distinct x y z)", out.str());

  s.add(d);

  EXPECT_EQ(sat, s.check());

  s.push();
  {
    s.add(x == y);
    EXPECT_EQ(unsat, s.check());
  }
  s.pop();

  s.push();
  {
    s.add(x == z);
    EXPECT_EQ(unsat, s.check());
  }
  s.pop();

  s.push();
  {
    s.add(y == z);
    EXPECT_EQ(unsat, s.check());
  }
  s.pop();

  s.push();
  {
    s.add(x == w);
    EXPECT_EQ(sat, s.check());
  }
  s.pop();

  s.push();
  {
    s.add(y == w);
    EXPECT_EQ(sat, s.check());
  }
  s.pop();

  s.push();
  {
    s.add(z == w);
    EXPECT_EQ(sat, s.check());
  }
  s.pop();
}

TEST(SmtZ3Test, LogicalImplication)
{
  Z3Solver s;
  z3::expr expr(s.context());

  const Decl<Bool> d0("x");
  const Decl<Bool> d1("y");
  const Bool e0_term = constant(d0);
  const Bool e1_term = constant(d1);
  const BinaryExpr<IMP, Bool> e2(e0_term, e1_term);

  EXPECT_EQ(OK, e2.encode(s));

  expr = s.expr();
  EXPECT_TRUE(expr.is_bool());
  EXPECT_TRUE(expr.is_app());

  z3::solver& solver = s.solver();

  const z3::expr x_expr(s.context().bool_const("x"));
  const z3::expr y_expr(s.context().bool_const("y"));
  solver.push();
  {
    solver.add(implies(x_expr, y_expr) == expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(implies(x_expr, y_expr) != expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, UnaryFuncAppExpr)
{
  Z3Solver s;

  Decl<Func<Int, Bv<long>>> d0("f");
  const Int e1_term(new LiteralExpr<Int, int>(7));
  const FuncAppExpr<Int, Bv<long>> e2(d0, std::make_tuple(e1_term));

  EXPECT_EQ(OK, e2.encode(s));
  const z3::expr app_expr(s.expr());

  z3::context& context = s.context();
  z3::solver& solver = s.solver();

  const z3::sort long_bv_sort(context.bv_sort(sizeof(long) * 8));
  const z3::func_decl f_decl(context.function("f", context.int_sort(), long_bv_sort));

  solver.push();
  {
    solver.add(f_decl(3) == app_expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(f_decl(3) != app_expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(f_decl(7) == app_expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(f_decl(7) != app_expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, BinaryFuncAppExpr)
{
  Z3Solver s;

  const Decl<Func<Int, Bv<bool>, Bv<long>>> d0("f");
  const Decl<Bv<bool>> d2("x");
  const Int e1_term(new LiteralExpr<Int, int>(7));
  const Bv<bool> e2_term = constant(d2);
  const FuncAppExpr<Int, Bv<bool>, Bv<long>> e3(d0, std::make_tuple(e1_term, e2_term));

  EXPECT_EQ(OK, e3.encode(s));
  const z3::expr app_expr(s.expr());

  z3::context& context = s.context();
  z3::solver& solver = s.solver();

  const z3::sort bool_bv_sort(context.bv_sort(sizeof(bool) * 8));
  const z3::sort long_bv_sort(context.bv_sort(sizeof(long) * 8));
  const z3::func_decl f_decl(context.function("f", context.int_sort(),
    bool_bv_sort, long_bv_sort));

  const z3::expr x_expr(s.context().constant("x", bool_bv_sort));
  solver.push();
  {
    solver.add(f_decl(3, x_expr) == app_expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(f_decl(3, x_expr) != app_expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(f_decl(7, x_expr) == app_expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(f_decl(7, x_expr) != app_expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, ConstArrayExpr)
{
  Z3Solver s;

  const Int init_term(new LiteralExpr<Int, int>(7));
  const ConstArrayExpr<Int, Int> e0(init_term);

  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr const_array_expr(s.expr());
  EXPECT_TRUE(const_array_expr.is_app());
  EXPECT_TRUE(const_array_expr.is_array());

  z3::context& context = s.context();
  z3::solver& solver = s.solver();

  const z3::expr i_expr(context.int_const("i"));
  const z3::expr j_expr(context.int_const("j"));

  solver.push();
  {
    solver.add(z3::select(const_array_expr, i_expr) == 7);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(z3::select(const_array_expr, i_expr) != 7);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(z3::select(const_array_expr, i_expr) != z3::select(const_array_expr, j_expr));
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, ArraySelectExpr)
{
  Z3Solver s;

  constexpr char const array_name[] = "array";
  const Decl<Array<Int, Bool>> array_decl(array_name);
  const Decl<Int> index_decl("i");
  const Array<Int, Bool> array_term = constant(array_decl);
  const Int index_term = constant(index_decl);
  const ArraySelectExpr<Int, Bool> e0(array_term, index_term);

  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr select_expr(s.expr());
  EXPECT_TRUE(select_expr.is_app());
  EXPECT_TRUE(select_expr.is_bool());

  z3::context& context = s.context();
  z3::solver& solver = s.solver();

  solver.add(context.bool_val(true) == select_expr);
  EXPECT_EQ(z3::sat, solver.check());

  const z3::sort array_sort = context.array_sort(context.int_sort(), context.bool_sort());
  const z3::expr array_expr = context.constant(array_name, array_sort);
  const z3::expr i_expr(context.int_const("i"));
  const z3::expr j_expr(context.int_const("j"));
  const z3::expr store_expr = z3::store(array_expr, j_expr, context.bool_val(false));
  solver.add(array_expr == store_expr);
  EXPECT_EQ(z3::sat, solver.check());

  solver.add(i_expr == j_expr);
  EXPECT_EQ(z3::unsat, solver.check());
}

TEST(SmtZ3Test, ArrayStoreExpr)
{
  Z3Solver s;

  const Decl<Array<Int, Int>> array_decl("array");
  const Decl<Int> index_decl("i");
  const Array<Int, Int> array_term = constant(array_decl);
  const Int index_term = constant(index_decl);
  const Int value_term(new LiteralExpr<Int, int>(7));
  const ArrayStoreExpr<Int, Int> e0(array_term, index_term, value_term);

  EXPECT_EQ(OK, e0.encode(s));

  const z3::expr store_expr(s.expr());
  EXPECT_TRUE(store_expr.is_app());
  EXPECT_TRUE(store_expr.is_array());

  z3::context& context = s.context();
  z3::solver& solver = s.solver();

  const z3::expr i_expr(context.int_const("i"));
  const z3::expr j_expr(context.int_const("j"));

  solver.push();
  {
    solver.add(z3::select(store_expr, i_expr) == 7);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(z3::select(store_expr, i_expr) != 7);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(z3::select(store_expr, j_expr) == 7);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(z3::select(store_expr, j_expr) != 7);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, Add)
{
  Z3Solver s;

  const Bv<long> e0_term = any<Bv<long>>("x");
  s.add(0 < e0_term);

  const Int e1_term = any<Int>("y");
  s.add(0 < e1_term);

  z3::context& context = s.context();
  z3::solver& solver = s.solver();

  EXPECT_EQ(z3::sat, solver.check());

  const z3::expr x_expr(context.bv_const("x", sizeof(long) * 8));
  const z3::expr y_expr(context.int_const("y"));

  solver.push();
  {
    solver.add(3 == x_expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();

  {
    solver.add(0 > x_expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(3 == y_expr);
    EXPECT_EQ(z3::sat, solver.check());
  }
  solver.pop();

  solver.push();
  {
    solver.add(0 > y_expr);
    EXPECT_EQ(z3::unsat, solver.check());
  }
  solver.pop();
}

TEST(SmtZ3Test, BinaryBvSignedOperatorLSS)
{
  Z3Solver solver;

  Bv<int8_t> x = any<Bv<int8_t>>("x");
  solver.add('\0' < x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(bvslt #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvUnsignedOperatorLSS)
{
  Z3Solver solver;

  Bv<uint8_t> x = any<Bv<uint8_t>>("x");
  solver.add('\0' < x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(bvult #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvSignedOperatorGTR)
{
  Z3Solver solver;

  Bv<int8_t> x = any<Bv<int8_t>>("x");
  solver.add('\0' > x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(bvsgt #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvUnsignedOperatorGTR)
{
  Z3Solver solver;

  Bv<uint8_t> x = any<Bv<uint8_t>>("x");
  solver.add('\0' > x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(bvugt #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvSignedOperatorNEQ)
{
  Z3Solver solver;

  Bv<int8_t> x = any<Bv<int8_t>>("x");
  solver.add('\0' != x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(distinct #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvUnsignedOperatorNEQ)
{
  Z3Solver solver;

  Bv<uint8_t> x = any<Bv<uint8_t>>("x");
  solver.add('\0' != x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(distinct #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvSignedOperatorLEQ)
{
  Z3Solver solver;

  Bv<int8_t> x = any<Bv<int8_t>>("x");
  solver.add('\0' <= x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(bvsle #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvUnsignedOperatorLEQ)
{
  Z3Solver solver;

  Bv<uint8_t> x = any<Bv<uint8_t>>("x");
  solver.add('\0' <= x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(bvule #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvSignedOperatorGEQ)
{
  Z3Solver solver;

  Bv<int8_t> x = any<Bv<int8_t>>("x");
  solver.add('\0' >= x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(bvsge #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryBvUnsignedOperatorGEQ)
{
  Z3Solver solver;

  Bv<uint8_t> x = any<Bv<uint8_t>>("x");
  solver.add('\0' >= x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(bvuge #x00 x)", out.str());
}

TEST(SmtZ3Test, BinaryIntOperatorLSS)
{
  Z3Solver solver;

  Int x = any<Int>("x");
  solver.add(0 < x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(< 0 x)", out.str());
}

TEST(SmtZ3Test, BinaryIntOperatorGTR)
{
  Z3Solver solver;

  Int x = any<Int>("x");
  solver.add(0 > x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(> 0 x)", out.str());
}

TEST(SmtZ3Test, BinaryIntOperatorNEQ)
{
  Z3Solver solver;

  Int x = any<Int>("x");
  solver.add(0 != x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(distinct 0 x)", out.str());
}

TEST(SmtZ3Test, BinaryIntOperatorLEQ)
{
  Z3Solver solver;

  Int x = any<Int>("x");
  solver.add(0 <= x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(<= 0 x)", out.str());
}

TEST(SmtZ3Test, BinaryIntOperatorGEQ)
{
  Z3Solver solver;

  Int x = any<Int>("x");
  solver.add(0 >= x);

  std::stringstream out;
  out << solver.expr();
  EXPECT_EQ("(>= 0 x)", out.str());
}

TEST(SmtZ3Test, AutoConfig)
{
  Z3Solver solver;

  auto x = any<Bv<long>>("x");
  solver.add(0 < x);

  auto y = any<Int>("y");
  solver.add(0 < y);

  EXPECT_EQ(smt::sat, solver.check());

  solver.push();
  {
    solver.add(3 == x);
    EXPECT_EQ(smt::sat, solver.check());
  }
  solver.pop();

  EXPECT_EQ(smt::sat, solver.check());

  solver.push();

  {
    solver.add(0 > x);
    EXPECT_EQ(smt::unsat, solver.check());
  }
  solver.pop();

  EXPECT_EQ(smt::sat, solver.check());

  solver.push();
  {
    solver.add(3 == y);
    EXPECT_EQ(smt::sat, solver.check());
  }
  solver.pop();

  EXPECT_EQ(smt::sat, solver.check());

  solver.push();
  {
    solver.add(0 > y);
    EXPECT_EQ(smt::unsat, solver.check());
  }
  solver.pop();

  EXPECT_EQ(smt::sat, solver.check());
}

TEST(SmtZ3Test, QF_BV)
{
  Z3Solver solver(QF_BV_LOGIC);

  auto x = any<Bv<long>>("x");
  solver.add(0 < x);

  EXPECT_EQ(smt::sat, solver.check());

  solver.push();
  {
    solver.add(3 == x);
    EXPECT_EQ(smt::sat, solver.check());
  }
  solver.pop();

  EXPECT_EQ(smt::sat, solver.check());

  solver.push();

  {
    solver.add(0 > x);
    EXPECT_EQ(smt::unsat, solver.check());
  }
  solver.pop();

  EXPECT_EQ(smt::sat, solver.check());
}

TEST(SmtZ3Test, QF_IDL)
{
  Z3Solver solver(QF_IDL_LOGIC);

  auto y = any<Int>("y");
  solver.add(0 < y);

  EXPECT_EQ(smt::sat, solver.check());

  solver.push();
  {
    solver.add(3 == y);
    EXPECT_EQ(smt::sat, solver.check());
  }
  solver.pop();

  EXPECT_EQ(smt::sat, solver.check());

  solver.push();
  {
    solver.add(0 > y);
    EXPECT_EQ(smt::unsat, solver.check());
  }
  solver.pop();

  EXPECT_EQ(smt::sat, solver.check());
}

TEST(SmtZ3Test, Reset)
{
  Z3Solver s;

  EXPECT_EQ(sat, s.check());
  s.add(literal<Bool>(false));
  EXPECT_EQ(unsat, s.check());
  s.reset();
  EXPECT_EQ(sat, s.check());
}

TEST(SmtZ3Test, UnsafeAdd)
{
  Z3Solver s;

  const Sort& bv_sort = internal::sort<Bv<int64_t>>();
  const Sort& func_sort = internal::sort<Func<Bv<int64_t>, Bv<int64_t>>>();
  const UnsafeDecl const_decl("x", bv_sort);
  const UnsafeDecl func_decl("f", func_sort);
  const UnsafeTerm seven_term(literal(bv_sort, 7));
  const UnsafeTerm x_term(constant(const_decl));
  const UnsafeTerm app_term(apply(func_decl, seven_term));

  UnsafeTerms terms;
  terms.push_back(seven_term);
  terms.push_back(x_term);
  terms.push_back(app_term);

  const UnsafeTerm distinct_term(distinct(std::move(terms)));

  const Sort& array_sort = internal::sort<Array<Bv<uint32_t>, Bv<int64_t>>>();
  const Sort& index_sort = internal::sort<Bv<uint32_t>>();
  const UnsafeDecl array_decl("array", array_sort);
  const UnsafeDecl index_decl("index", index_sort);
  const UnsafeTerm array_term(constant(array_decl));
  const UnsafeTerm index_term(constant(index_decl));
  const UnsafeTerm store_term(store(array_term, index_term, app_term));
  const UnsafeTerm select_term(select(store_term, index_term));

  const UnsafeTerm eq_term(select_term == x_term);
  const UnsafeTerm and_term(eq_term && distinct_term);

  s.push();
  {
    s.unsafe_add(and_term);
    EXPECT_EQ(unsat, s.check());

    std::stringstream out;
    out << s.expr();
    EXPECT_EQ("(let ((a!1 "
                 "(= (select (store array index (f #x0000000000000007)) index) x)))\n  "
             "(and a!1 (distinct #x0000000000000007 x (f #x0000000000000007))))", out.str());
  }
  s.pop();

  s.push();
  {
    s.unsafe_add(seven_term != 7);
    EXPECT_EQ(unsat, s.check());
  }
  s.pop();

  s.push();
  {
    s.unsafe_add(7 == seven_term);
    EXPECT_EQ(sat, s.check());
  }
  s.pop();

  s.push();
  {
    s.unsafe_add(x_term == x_term + 1);
    EXPECT_EQ(unsat, s.check());
  }
  s.pop();

  s.push();
  {
    s.unsafe_add(x_term + 1 == x_term);
    EXPECT_EQ(unsat, s.check());
  }
  s.pop();
}
