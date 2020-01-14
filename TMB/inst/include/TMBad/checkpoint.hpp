#ifndef HAVE_CHECKPOINT_HPP
#define HAVE_CHECKPOINT_HPP
// Autogenerated - do not edit by hand !
#define GLOBAL_HASH_TYPE unsigned int
#define GLOBAL_COMPRESS_TOL 16
#define GLOBAL_UNION_OR_STRUCT union
#define stringify(s) #s
#define xstringify(s) stringify(s)
#define THREAD_NUM 0
#define GLOBAL_INDEX_VECTOR std::vector<GLOBAL_INDEX_TYPE>
#define GLOBAL_INDEX_TYPE unsigned int
#define GLOBAL_MAX_NUM_THREADS 48
#define INDEX_OVERFLOW(x) \
  ((size_t)(x) >= (size_t)std::numeric_limits<GLOBAL_INDEX_TYPE>::max())
#define ASSERT(x)                                    \
  if (!(x)) {                                        \
    std::cout << "ASSERTION FAILED: " << #x << "\n"; \
    abort();                                         \
  }
#define GLOBAL_REPLAY_TYPE ad_aug
#define INHERIT_CTOR(A, B)                          \
  A() {}                                            \
  template <class T1>                               \
  A(T1 x1) : B(x1) {}                               \
  template <class T1, class T2>                     \
  A(T1 x1, T2 x2) : B(x1, x2) {}                    \
  template <class T1, class T2, class T3>           \
  A(T1 x1, T2 x2, T3 x3) : B(x1, x2, x3) {}         \
  template <class T1, class T2, class T3, class T4> \
  A(T1 x1, T2 x2, T3 x3, T4 x4) : B(x1, x2, x3, x4) {}
#define GLOBAL_SCALAR_TYPE double
#include "global.hpp"

namespace TMBad {

/** \brief Generic checkpoint operator
    \details This class implements checkpointing. It handles two cases:

    1. The **adaptive case** where the computational graph can change
   dynamically. The purpose of this case is to allow algorithms that use
   parameter dependendent branching.
    2. The **fixed graph case** where the computational graph doesn't change.
   The purpose of this case is to reduce memory when the same operation sequence
   is repeated many times.

    The two cases have different properties summarized in this table and
   explained below:

    | Property             | Adaptive case | Fixed graph case |
    |----------------------|---------------|------------------|
    | Retaping?            |   Yes         |    No            |
    | Shareable?           |   No          |    Yes           |
    | Re-do forward?       |   No          |    Yes           |
    | Parent needs child?  |   Yes         |    Yes           |
    | Child needs root?    |   Yes         |    No            |
    | Can be sparse?       |   No          |    Yes           |

    - **Retaping** means that that the computational graph of function *and*
   derivatives must be rebuilt on every new function evaluation.
    - **Shareable** means that a pointer to an instance of the operator can be
   safely shared without invoking a deep copy.
    - **Re-do forward** means that the workspace (global::values) used by the
   forward sweep may have been used by another operator instance which implies
   that the forward sweep must be re-calulated before doing a reverse sweep.
    - **Parent needs child** means that the function value operator (parent)
   must have a reference to its derivative operator (child).
    - **Child needs root** This property is best illustrated by an example. Say
   we start out with a tape including a checkpoint operator \f$F\f$ (the root).
   Denoting by 'I' and 'D' the *independent variable* and the *dependent
   variable* operators, the tape will look something like: \f[I \rightarrow ...
   \rightarrow F \rightarrow ... \rightarrow D \f] Say we retape a forward and
   reverse sweep of the sequence a couple of times: \f[I \rightarrow ...
   \rightarrow F \rightarrow ... \rightarrow F' \rightarrow ... \rightarrow D
   \f] \f[I \rightarrow ... \rightarrow F \rightarrow ... \rightarrow F'
   \rightarrow ... \rightarrow F''  \rightarrow ... \rightarrow F' \rightarrow
   ... \rightarrow D \f] Imagine a case where only \f$F''\f$ affects the output
   of the operation sequence and assume we run the tape optimizer. Now the tape
   will look something like: \f[I \rightarrow ... \rightarrow F'' \rightarrow
   ... \rightarrow D \f] This reduction would be problematic in the adaptive
   case because the root must remain on the tape in order to correctly rebuild
   all derivatives.
    - **Can be sparse** The adaptive case cannot use sparsity because it is
   impossible to analyze every possible branch of a general adaptive algorithm.
*/
template <bool retape = false>
struct CallOp_ : global::SharedDynamicOperator {
  /** \brief Function value tape */
  global* glob;
  /** \brief Derivative tape. If allocated, it is managed by
   * `global::operation_stack` */
  global::Complete<CallOp_>* Dglob;
  /** \brief Encapsulating Completion of 'this'. If allocated, it is managed by
   * `global::operation_stack` */
  global::OperatorPure* self;
  /** \brief Deep copy of `self` used to handle replay of adaptive operator
   * (which can't be shared - see table above) */
  global::OperatorPure* self_replay;
  typedef CallOp_<retape> Operator;
  /** \brief Force an allocated derivative to **not** go out of scope
      before its parent.
      \details If this function is called on every construction of a
      derivative `Dglob` it is guaranteed that deallocation of
      function and derivatives happens in the order
      \f$F \rightarrow F' \rightarrow F'' \rightarrow\f$
      regardless of whether some of these operators have been removed
      by the tape optimizer.
  */
  void protect_child() {
    if (Dglob != NULL) Dglob->copy();
  }
  /** \brief For every `protect_child()` there must be an
      `unprotect_child()` */
  void unprotect_child() {
    if (Dglob != NULL) Dglob->deallocate();
  }
  /** \brief Get next element of linked list */
  CallOp_* next_CallOp() { return &(Dglob->Op); }
  /** \brief Get next linked list element's tape */
  global* next_glob() { return Dglob->Op.glob; }
  /** \brief Get `global::Complete` version of derivative (so can be put on a
   * stack) */
  global::Complete<CallOp_>* next_Dglob() { return Dglob->Op.Dglob; }
  Index input_size() const { return glob->inv_index.size(); }
  Index output_size() const { return glob->dep_index.size(); }
  const char* op_name() { return "CallOp"; }
  void print(global::print_config cfg) { this->glob->print(cfg); }
  CallOp_(global glob) : Dglob(NULL), self_replay(NULL), root_var(-1) {
    this->glob = new global(glob);
  }
  CallOp_(global* glob)
      : glob(glob), Dglob(NULL), self_replay(NULL), root_var(-1) {}

  CallOp_() : glob(NULL), Dglob(NULL), self_replay(NULL), root_var(-1) {}

  CallOp_(const CallOp_& other) {
    this->glob = NULL;
    this->Dglob = NULL;
    if (other.glob != NULL) {
      this->glob = new global(*other.glob);
    }
  }
  CallOp_& operator=(const CallOp_& other) { ASSERT(false); }
  ~CallOp_() {
    if (glob != NULL) {
      delete glob;
      glob = NULL;
    }
    unprotect_child();
  }
  /** \brief The retape case **must** override the retape hook */
  virtual void retape_hook(ForwardArgs<Scalar>& args) {}
  /** \brief The retape case **must** override the replay hook */
  virtual void replay_hook(ForwardArgs<global::Replay>& args) {}
  /** \brief The retape case overrides the dependency markers */
  static const bool have_forward_mark_reverse_mark = retape;
  /** \brief Retape case needs access to the variable id of some
      component of function value *output* */
  Index root_var;
  /** \brief Used only in the retape case (see
   * `global::Operator::have_forward_mark_reverse_mark`) */
  void forward(ForwardArgs<bool>& args) {
    typedef typename global::CPL<CallOp_>::type CPL_CallOp;
    args.mark_dense<CPL_CallOp>(*this);
  };
  /** \brief Used only in the retape case (see
   * `global::Operator::have_forward_mark_reverse_mark`) */
  void reverse(ReverseArgs<bool>& args) {
    typedef typename global::CPL<CallOp_>::type CPL_CallOp;
    bool any_marked_y = args.mark_dense<CPL_CallOp>(*this);
    if (any_marked_y) {
      ASSERT(root_var != (Index)-1);
      if (root_var != (Index)-1) args.values[root_var] = true;
    }
  }
  /** \brief Helper to construct and update derivative glob */
  void update_deriv() {
    if (Dglob == NULL) {
      global* glob = new global();
      Dglob = new global::Complete<CallOp_>(glob);
      Dglob->Op.self = Dglob;
      protect_child();
      Dglob->Op.root_var = this->root_var;
    } else {
      if (next_glob() != NULL) next_glob()->clear();
    }

    global* newglob = next_glob();
    global::replay replay(*glob, *newglob);
    replay.start();
    replay.forward(true, false);
    replay.clear_deriv();
    replay.reverse(true, true);
    replay.stop();
    replay.target.eliminate();
  }

  void forward(ForwardArgs<Scalar>& args) {
    retape_hook(args);
    for (size_t i = 0; i < input_size(); i++) glob->value_inv(i) = args.x(i);

    glob->forward();
    for (size_t i = 0; i < output_size(); i++) args.y(i) = glob->value_dep(i);
  }
  void reverse(ReverseArgs<Scalar>& args) {
    if (!retape) {
      for (size_t i = 0; i < input_size(); i++) glob->value_inv(i) = args.x(i);
      glob->forward();
    }
    glob->clear_deriv();
    for (size_t i = 0; i < output_size(); i++) glob->deriv_dep(i) = args.dy(i);
    glob->reverse();
    for (size_t i = 0; i < input_size(); i++) args.dx(i) += glob->deriv_inv(i);
  }

  void forward(ForwardArgs<global::Replay>& args) {
    replay_hook(args);
    if (!retape) self_replay = self;
    std::vector<ad_plain> x(input_size());
    for (size_t i = 0; i < input_size(); i++) x[i] = args.x(i);
    std::vector<ad_plain> y =
        get_glob()->add_to_stack<Operator>(self_replay->copy(), x);
    for (size_t i = 0; i < output_size(); i++) args.y(i) = y[i];
  }
  void reverse(ReverseArgs<global::Replay>& args) {
    CallOp_* tmp = (CallOp_*)self_replay->incomplete();
    if (tmp->Dglob == NULL) tmp->update_deriv();
    std::vector<ad_plain> x;
    for (size_t i = 0; i < input_size(); i++) x.push_back(args.x(i));
    for (size_t i = 0; i < output_size(); i++) x.push_back(args.dy(i));
    std::vector<ad_plain> dx =
        get_glob()->add_to_stack<Operator>(tmp->Dglob->copy(), x);
    for (size_t i = 0; i < input_size(); i++) args.dx(i) += dx[i];
  }

  void forward(ForwardArgs<Writer>& args) { ASSERT(false); }
  void reverse(ReverseArgs<Writer>& args) { ASSERT(false); }
};

struct CallOp {
  typedef CallOp_<false> Operator;
  global::Complete<Operator>* pOp;
  bool in_use;
  CallOp(global glob);
  ~CallOp();
  std::vector<ad_plain> operator()(const std::vector<ad_plain>& x);
  ad_plain operator()(ad_plain x0);
  ad_plain operator()(ad_plain x0, ad_plain x1);
  ad_plain operator()(ad_plain x0, ad_plain x1, ad_plain x2);
};

/** \brief Container of adaptive algorithms
    \details The adaptive algoritm is passed as a functor. It has the following
   properties:
    - Adaptive algorithms cannot be shared *within* a tape. A deep
      copy is triggered if an adaptive operator is replayed. The deep
      copy includes the entire linked list of allocated function value
      and derivatives.
    - Adaptive operators are allowed to be shared *among* tapes. This
      happens if an operation sequence including adaptive operators is
      copied. Reference counting makes this a relatively fast
      operation.
    - Adaptive operators are protected from the tape optimizer as long
      as the tape contains any of its derivatives.
*/
template <class Functor, class ad = ad_plain>
struct AdapOp_ : CallOp_<true> {
  Functor F;
  AdapOp_(Functor F) : F(F) {}
  /** \brief Retape helper */
  void retape(std::vector<Scalar>& x_) {
    if (this->glob == NULL) this->glob = new global();
    this->glob->clear();
    this->glob->ad_start();
    std::vector<ad> x(x_.begin(), x_.end());
    Independent(x);
    std::vector<ad> y = F(x);
    Dependent(y);
    this->glob->ad_stop();
    this->glob->eliminate();
  }
  /** Overrides the virtual `retape_hook` of Base class
      \details Retapes the functor and traverses the linked list of
      derivatives. Each derivative tape is updated and a dependency on
      the root variable is added to protect this adaptive operator
      from the tape optimizer.
  */
  void retape_hook(ForwardArgs<Scalar>& args) {
    std::vector<Scalar> x(this->input_size());
    for (size_t i = 0; i < x.size(); i++) x[i] = args.x(i);
    retape(x);

    CallOp_<true>* current = this;

    if (this->output_size() > 0) current->root_var = args.output(0);
    while (current->Dglob != NULL) {
      current->update_deriv();
      current = current->next_CallOp();
    }
  }
  /** Overrides the virtual `replay_hook` of Base class
      \details Allocates a deep copy of this adaptive operator
      including its entire linked list of derivatives. These copies
      are used to replay rather than the originals.
  */
  void replay_hook(ForwardArgs<global::Replay>& args) {
    global::Complete<AdapOp_>* pOp = new global::Complete<AdapOp_>(*this);
    pOp->Op.self = pOp;

    CallOp_<true>* current = this;
    CallOp_<true>* current_ = &(pOp->Op);
    current->self_replay = current_->self;

    if (this->output_size() > 0) current_->root_var = args.output(0);
    while (current->Dglob != NULL) {
      current_->update_deriv();
      current_ = current_->next_CallOp();
      current = current->next_CallOp();
      current->self_replay = current_->self;
    }
  }
  const char* op_name() { return "AdapOp"; }
};

template <class Functor, class ad = ad_plain>
struct AdapOp {
  typedef AdapOp_<Functor, ad> Operator;
  typedef global::Complete<Operator> CPL_Operator;
  Functor F;
  AdapOp(Functor F) : F(F) {}

  std::vector<ad_plain> operator()(std::vector<ad_plain>& x) {
    CPL_Operator* pOp = new CPL_Operator(F);
    pOp->Op.self = pOp;
    std::vector<Scalar> xd(x.size());
    for (size_t i = 0; i < x.size(); i++) xd[i] = x[i].Value();
    pOp->Op.retape(xd);
    return get_glob()->add_to_stack<Operator>(pOp->copy(), x);
  }
  ad_plain operator()(ad_plain x0) {
    std::vector<ad_plain> tmp(1);
    tmp[0] = x0;
    return this->operator()(tmp)[0];
  }
  ad_plain operator()(ad_plain x0, ad_plain x1) {
    std::vector<ad_plain> tmp(2);
    tmp[0] = x0;
    tmp[1] = x1;
    return this->operator()(tmp)[0];
  }
  ad_plain operator()(ad_plain x0, ad_plain x1, ad_plain x2) {
    std::vector<ad_plain> tmp(3);
    tmp[0] = x0;
    tmp[1] = x1;
    tmp[2] = x2;
    return this->operator()(tmp)[0];
  }
};

}  // namespace TMBad
#endif  // HAVE_CHECKPOINT_HPP
