// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/QAO.hpp>

#include <gtest/gtest.h>

using namespace hg::qao;

// MARK: PriorityResolver

TEST(QAO_PriorityResolverTest, PriorityResolverBasics) {
    enum Categories {
        A,
        B,
        C
    };

    QAO_PriorityResolver resolver;
    resolver.category(A);
    resolver.category(B).dependsOn(A);
    resolver.category(C).dependsOn(A, B);
    resolver.resolveAll();

    ASSERT_GT(resolver.getPriorityOf(A), resolver.getPriorityOf(B)); // A before B
    ASSERT_GT(resolver.getPriorityOf(B), resolver.getPriorityOf(C)); // B before C
}

TEST(QAO_PriorityResolverTest, BasicsWithPriorityStartAndStep) {
    enum Categories {
        A,
        B,
        C
    };

    QAO_PriorityResolver resolver{500, 50};
    resolver.category(A);
    resolver.category(B).dependsOn(A);
    resolver.category(C).dependsOn(A, B);
    resolver.resolveAll();

    ASSERT_EQ(resolver.getPriorityOf(A), 500);
    ASSERT_EQ(resolver.getPriorityOf(B), 450);
    ASSERT_EQ(resolver.getPriorityOf(C), 400);
}

TEST(QAO_PriorityResolverTest, WithEnumClass) {
    enum Categories {
        A,
        B,
        C
    };

    QAO_PriorityResolver resolver;
    resolver.category(Categories::A);
    resolver.category(Categories::B).dependsOn(Categories::A);
    resolver.category(Categories::C).dependsOn(Categories::A, Categories::B);
    resolver.resolveAll();

    ASSERT_GT(resolver.getPriorityOf(Categories::A),
              resolver.getPriorityOf(Categories::B)); // A before B
    ASSERT_GT(resolver.getPriorityOf(Categories::B),
              resolver.getPriorityOf(Categories::C)); // B before C
}

TEST(QAO_PriorityResolverTest, ReversedInput) {
    enum Categories {
        A,
        B,
        C
    };

    QAO_PriorityResolver resolver;
    resolver.category(C).dependsOn(A, B);
    resolver.category(B).dependsOn(A);
    resolver.category(A);

    resolver.resolveAll();

    ASSERT_GT(resolver.getPriorityOf(A), resolver.getPriorityOf(B)); // A before B
    ASSERT_GT(resolver.getPriorityOf(B), resolver.getPriorityOf(C)); // B before C
}

TEST(QAO_PriorityResolverTest, ImpossibleCycle) {
    enum Categories {
        A,
        B,
        C
    };

    QAO_PriorityResolver resolver;
    resolver.category(A).dependsOn(C);
    resolver.category(B).dependsOn(A);
    resolver.category(C).dependsOn(B);

    ASSERT_THROW(resolver.resolveAll(), hg::TracedLogicError);
}

TEST(QAO_PriorityResolverTest, MissingDefinition) {
    enum Categories {
        A,
        B
    };

    QAO_PriorityResolver resolver;
    resolver.category(A).dependsOn(B);

    ASSERT_THROW(resolver.resolveAll(), hg::TracedLogicError);
}

TEST(QAO_PriorityResolverTest, ComplexResolve) {
    enum Categories {
        A,
        B,
        C,
        D
    };

    QAO_PriorityResolver resolver;
    resolver.category(B);
    resolver.category(C).dependsOn(B);
    resolver.category(A).dependsOn(B, C);
    resolver.category(D).dependsOn(C, A);

    resolver.resolveAll();

    ASSERT_GT(resolver.getPriorityOf(B), resolver.getPriorityOf(C)); // B before C
    ASSERT_GT(resolver.getPriorityOf(C), resolver.getPriorityOf(A)); // C before A
    ASSERT_GT(resolver.getPriorityOf(A), resolver.getPriorityOf(D)); // A before D
}

// MARK: PriorityResolver2

TEST(QAO_PriorityResolver2Test, Basics) {
    int A = 1000, B = 1000, C = 1000;

    QAO_PriorityResolver2 resolver;
    resolver.category(&A);
    resolver.category(&B).dependsOn(&A);
    resolver.category(&C).dependsOn(&A, &B);
    resolver.resolveAll();

    ASSERT_GT(A, B); // A before B
    ASSERT_GT(B, C); // B before C
}

TEST(QAO_PriorityResolver2Test, BasicsWithPriorityStartAndStep) {
    int A = 1000, B = 1000, C = 1000;

    QAO_PriorityResolver2 resolver{500, 50};
    resolver.category(&A);
    resolver.category(&B).dependsOn(&A);
    resolver.category(&C).dependsOn(&A, &B);
    resolver.resolveAll();

    ASSERT_EQ(A, 500);
    ASSERT_EQ(B, 450);
    ASSERT_EQ(C, 400);
}

TEST(QAO_PriorityResolver2Test, ReversedInput) {
    int A = 1000, B = 1000, C = 1000;

    QAO_PriorityResolver2 resolver;
    resolver.category(&C).dependsOn(&A, &B);
    resolver.category(&B).dependsOn(&A);
    resolver.category(&A);

    resolver.resolveAll();

    ASSERT_GT(A, B); // A before B
    ASSERT_GT(B, C); // B before C
}

TEST(QAO_PriorityResolver2Test, ImpossibleCycle) {
    int A = 1000, B = 1000, C = 1000;

    QAO_PriorityResolver2 resolver;
    resolver.category(&A).dependsOn(&C);
    resolver.category(&B).dependsOn(&A);
    resolver.category(&C).dependsOn(&B);

    ASSERT_THROW(resolver.resolveAll(), hg::TracedLogicError);
}

TEST(QAO_PriorityResolver2Test, MissingDefinition) {
    int A = 1000, B = 1000;

    QAO_PriorityResolver2 resolver;
    resolver.category(&A).dependsOn(&B);

    resolver.resolveAll();

    ASSERT_GT(B, A); // B before A
}

TEST(QAO_PriorityResolver2Test, ComplexResolve) {
    int A = 1000, B = 1000, C = 1000, D = 1000;

    QAO_PriorityResolver2 resolver;
    resolver.category(&B);
    resolver.category(&C).dependsOn(&B);
    resolver.category(&A).dependsOn(&B, &C);
    resolver.category(&D).dependsOn(&C, &A);

    resolver.resolveAll();

    ASSERT_GT(B, C); // B before C
    ASSERT_GT(C, A); // C before A
    ASSERT_GT(A, D); // A before D
}

TEST(QAO_PriorityResolver2Test, PrecedesMethod) {
    int A = 1000, B = 1000, C = 1000, D = 1000;

    QAO_PriorityResolver2 resolver;
    resolver.category(&B).precedes(&A);
    resolver.category(&C).dependsOn(&B).precedes(&A, &D);
    resolver.category(&D).dependsOn(&C, &A);

    resolver.resolveAll();

    ASSERT_GT(B, C); // B before C
    ASSERT_GT(C, A); // C before A
    ASSERT_GT(A, D); // A before D
}
