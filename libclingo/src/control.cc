#include "clingo.hh"
#include "gringo/control.hh"

using namespace Gringo;

namespace {

template <class S, class P, class ...Args>
std::string to_string(S size, P print, Args ...args) {
    std::vector<char> ret;
    size_t n;
    handleCError(size(std::forward<Args>(args)..., &n));
    ret.resize(n);
    handleCError(print(std::forward<Args>(args)..., ret.data(), n));
    return std::string(ret.begin(), ret.end()-1);
}

} // namespace

namespace Clingo {

// c++ interface

// {{{1 signature

Signature::Signature(char const *name, uint32_t arity, bool positive) {
    handleCError(clingo_signature_create(name, arity, positive, &sig_));
}

char const *Signature::name() const {
    return clingo_signature_name(sig_);
}

uint32_t Signature::arity() const {
    return clingo_signature_arity(sig_);
}

bool Signature::positive() const {
    return clingo_signature_is_positive(sig_);
}

bool Signature::negative() const {
    return clingo_signature_is_negative(sig_);
}

size_t Signature::hash() const {
    return clingo_signature_hash(sig_);
}

bool operator==(Signature a, Signature b) { return  clingo_signature_is_equal_to(a.to_c(), b.to_c()); }
bool operator!=(Signature a, Signature b) { return !clingo_signature_is_equal_to(a.to_c(), b.to_c()); }
bool operator< (Signature a, Signature b) { return  clingo_signature_is_less_than(a.to_c(), b.to_c()); }
bool operator<=(Signature a, Signature b) { return !clingo_signature_is_less_than(b.to_c(), a.to_c()); }
bool operator> (Signature a, Signature b) { return  clingo_signature_is_less_than(b.to_c(), a.to_c()); }
bool operator>=(Signature a, Signature b) { return !clingo_signature_is_less_than(a.to_c(), b.to_c()); }

// {{{1 symbol

Symbol::Symbol() {
    clingo_symbol_create_number(0, &sym_);
}

Symbol::Symbol(clingo_symbol_t sym)
: sym_(sym) { }

Symbol Number(int num) {
    clingo_symbol_t sym;
    clingo_symbol_create_number(num, &sym);
    return Symbol(sym);
}

Symbol Supremum() {
    clingo_symbol_t sym;
    clingo_symbol_create_supremum(&sym);
    return Symbol(sym);
}

Symbol Infimum() {
    clingo_symbol_t sym;
    clingo_symbol_create_infimum(&sym);
    return Symbol(sym);
}

Symbol String(char const *str) {
    clingo_symbol_t sym;
    handleCError(clingo_symbol_create_string(str, &sym));
    return Symbol(sym);
}

Symbol Id(char const *id, bool positive) {
    clingo_symbol_t sym;
    handleCError(clingo_symbol_create_id(id, positive, &sym));
    return Symbol(sym);
}

Symbol Function(char const *name, SymbolSpan args, bool positive) {
    clingo_symbol_t sym;
    handleCError(clingo_symbol_create_function(name, reinterpret_cast<clingo_symbol_t const *>(args.begin()), args.size(), positive, &sym));
    return Symbol(sym);
}

int Symbol::number() const {
    int ret;
    handleCError(clingo_symbol_number(sym_, &ret));
    return ret;
}

char const *Symbol::name() const {
    char const *ret;
    handleCError(clingo_symbol_name(sym_, &ret));
    return ret;
}

char const *Symbol::string() const {
    char const *ret;
    handleCError(clingo_symbol_string(sym_, &ret));
    return ret;
}

bool Symbol::is_positive() const {
    bool ret;
    handleCError(clingo_symbol_is_positive(sym_, &ret));
    return ret;
}

bool Symbol::is_negative() const {
    bool ret;
    handleCError(clingo_symbol_is_negative(sym_, &ret));
    return ret;
}

SymbolSpan Symbol::arguments() const {
    clingo_symbol_t const *ret;
    size_t n;
    handleCError(clingo_symbol_arguments(sym_, &ret, &n));
    return {reinterpret_cast<Symbol const *>(ret), n};
}

SymbolType Symbol::type() const {
    return static_cast<SymbolType>(clingo_symbol_type(sym_));
}

#define CLINGO_CALLBACK_TRY try
#define CLINGO_CALLBACK_CATCH(ref) catch (...){ (ref) = std::current_exception(); return false; } return true

std::string Symbol::to_string() const {
    return ::to_string(clingo_symbol_to_string_size, clingo_symbol_to_string, sym_);
}

size_t Symbol::hash() const {
    return clingo_symbol_hash(sym_);
}

std::ostream &operator<<(std::ostream &out, Symbol sym) {
    out << sym.to_string();
    return out;
}

bool operator==(Symbol a, Symbol b) { return  clingo_symbol_is_equal_to(a.to_c(), b.to_c()); }
bool operator!=(Symbol a, Symbol b) { return !clingo_symbol_is_equal_to(a.to_c(), b.to_c()); }
bool operator< (Symbol a, Symbol b) { return  clingo_symbol_is_less_than(a.to_c(), b.to_c()); }
bool operator<=(Symbol a, Symbol b) { return !clingo_symbol_is_less_than(b.to_c(), a.to_c()); }
bool operator> (Symbol a, Symbol b) { return  clingo_symbol_is_less_than(b.to_c(), a.to_c()); }
bool operator>=(Symbol a, Symbol b) { return !clingo_symbol_is_less_than(a.to_c(), b.to_c()); }

// {{{1 symbolic atoms

Symbol SymbolicAtom::symbol() const {
    clingo_symbol_t ret;
    clingo_symbolic_atoms_symbol(atoms_, range_, &ret);
    return Symbol(ret);
}

clingo_literal_t SymbolicAtom::literal() const {
    clingo_literal_t ret;
    clingo_symbolic_atoms_literal(atoms_, range_, &ret);
    return ret;
}

bool SymbolicAtom::is_fact() const {
    bool ret;
    clingo_symbolic_atoms_is_fact(atoms_, range_, &ret);
    return ret;
}

bool SymbolicAtom::is_external() const {
    bool ret;
    clingo_symbolic_atoms_is_external(atoms_, range_, &ret);
    return ret;
}

SymbolicAtomIterator &SymbolicAtomIterator::operator++() {
    clingo_symbolic_atom_iterator_t range;
    handleCError(clingo_symbolic_atoms_next(atoms_, range_, &range));
    range_ = range;
    return *this;
}

SymbolicAtomIterator::operator bool() const {
    bool ret;
    handleCError(clingo_symbolic_atoms_is_valid(atoms_, range_, &ret));
    return ret;
}

bool SymbolicAtomIterator::operator==(SymbolicAtomIterator it) const {
    bool ret = atoms_ == it.atoms_;
    if (ret) { handleCError(clingo_symbolic_atoms_iterator_is_equal_to(atoms_, range_, it.range_, &ret)); }
    return ret;
}

SymbolicAtomIterator SymbolicAtoms::begin() const {
    clingo_symbolic_atom_iterator_t it;
    handleCError(clingo_symbolic_atoms_begin(atoms_, nullptr, &it));
    return SymbolicAtomIterator{atoms_,  it};
}

SymbolicAtomIterator SymbolicAtoms::begin(Signature sig) const {
    clingo_symbolic_atom_iterator_t it;
    handleCError(clingo_symbolic_atoms_begin(atoms_, &sig.to_c(), &it));
    return SymbolicAtomIterator{atoms_, it};
}

SymbolicAtomIterator SymbolicAtoms::end() const {
    clingo_symbolic_atom_iterator_t it;
    handleCError(clingo_symbolic_atoms_end(atoms_, &it));
    return SymbolicAtomIterator{atoms_, it};
}

SymbolicAtomIterator SymbolicAtoms::find(Symbol atom) const {
    clingo_symbolic_atom_iterator_t it;
    handleCError(clingo_symbolic_atoms_find(atoms_, atom.to_c(), &it));
    return SymbolicAtomIterator{atoms_, it};
}

std::vector<Signature> SymbolicAtoms::signatures() const {
    size_t n;
    clingo_symbolic_atoms_signatures_size(atoms_, &n);
    Signature sig("", 0);
    std::vector<Signature> ret;
    ret.resize(n, sig);
    handleCError(clingo_symbolic_atoms_signatures(atoms_, reinterpret_cast<clingo_signature_t *>(ret.data()), n));
    return ret;
}

size_t SymbolicAtoms::length() const {
    size_t ret;
    handleCError(clingo_symbolic_atoms_size(atoms_, &ret));
    return ret;
}

// {{{1 theory atoms

TheoryTermType TheoryTerm::type() const {
    clingo_theory_term_type_t ret;
    handleCError(clingo_theory_atoms_term_type(atoms_, id_, &ret));
    return static_cast<TheoryTermType>(ret);
}

int TheoryTerm::number() const {
    int ret;
    handleCError(clingo_theory_atoms_term_number(atoms_, id_, &ret));
    return ret;
}

char const *TheoryTerm::name() const {
    char const *ret;
    handleCError(clingo_theory_atoms_term_name(atoms_, id_, &ret));
    return ret;
}

TheoryTermSpan TheoryTerm::arguments() const {
    clingo_id_t const *ret;
    size_t n;
    handleCError(clingo_theory_atoms_term_arguments(atoms_, id_, &ret, &n));
    return {ret, n, ToTheoryIterator<TheoryTermIterator>{atoms_}};
}

std::ostream &operator<<(std::ostream &out, TheoryTerm term) {
    out << term.to_string();
    return out;
}

std::string TheoryTerm::to_string() const {
    return ::to_string(clingo_theory_atoms_term_to_string_size, clingo_theory_atoms_term_to_string, atoms_, id_);
}


TheoryTermSpan TheoryElement::tuple() const {
    clingo_id_t const *ret;
    size_t n;
    handleCError(clingo_theory_atoms_element_tuple(atoms_, id_, &ret, &n));
    return {ret, n, ToTheoryIterator<TheoryTermIterator>{atoms_}};
}

LiteralSpan TheoryElement::condition() const {
    clingo_literal_t const *ret;
    size_t n;
    handleCError(clingo_theory_atoms_element_condition(atoms_, id_, &ret, &n));
    return {ret, n};
}

literal_t TheoryElement::condition_id() const {
    clingo_literal_t ret;
    handleCError(clingo_theory_atoms_element_condition_id(atoms_, id_, &ret));
    return ret;
}

std::string TheoryElement::to_string() const {
    return ::to_string(clingo_theory_atoms_element_to_string_size, clingo_theory_atoms_element_to_string, atoms_, id_);
}

std::ostream &operator<<(std::ostream &out, TheoryElement term) {
    out << term.to_string();
    return out;
}

TheoryElementSpan TheoryAtom::elements() const {
    clingo_id_t const *ret;
    size_t n;
    handleCError(clingo_theory_atoms_atom_elements(atoms_, id_, &ret, &n));
    return {ret, n, ToTheoryIterator<TheoryElementIterator>{atoms_}};
}

TheoryTerm TheoryAtom::term() const {
    clingo_id_t ret;
    handleCError(clingo_theory_atoms_atom_term(atoms_, id_, &ret));
    return TheoryTerm{atoms_, ret};
}

bool TheoryAtom::has_guard() const {
    bool ret;
    handleCError(clingo_theory_atoms_atom_has_guard(atoms_, id_, &ret));
    return ret;
}

literal_t TheoryAtom::literal() const {
    clingo_literal_t ret;
    handleCError(clingo_theory_atoms_atom_literal(atoms_, id_, &ret));
    return ret;
}

std::pair<char const *, TheoryTerm> TheoryAtom::guard() const {
    char const *name;
    clingo_id_t term;
    handleCError(clingo_theory_atoms_atom_guard(atoms_, id_, &name, &term));
    return {name, TheoryTerm{atoms_, term}};
}

std::string TheoryAtom::to_string() const {
    return ::to_string(clingo_theory_atoms_atom_to_string_size, clingo_theory_atoms_atom_to_string, atoms_, id_);
}

std::ostream &operator<<(std::ostream &out, TheoryAtom term) {
    out << term.to_string();
    return out;
}

TheoryAtomIterator TheoryAtoms::begin() const {
    return TheoryAtomIterator{atoms_, 0};
}

TheoryAtomIterator TheoryAtoms::end() const {
    return TheoryAtomIterator{atoms_, clingo_id_t(size())};
}

size_t TheoryAtoms::size() const {
    size_t ret;
    handleCError(clingo_theory_atoms_size(atoms_, &ret));
    return ret;
}

// {{{1 assignment

bool Assignment::has_conflict() const {
    return clingo_assignment_has_conflict(ass_);
}

uint32_t Assignment::decision_level() const {
    return clingo_assignment_decision_level(ass_);
}

bool Assignment::has_literal(literal_t lit) const {
    return clingo_assignment_has_literal(ass_, lit);
}

TruthValue Assignment::truth_value(literal_t lit) const {
    clingo_truth_value_t ret;
    handleCError(clingo_assignment_truth_value(ass_, lit, &ret));
    return static_cast<TruthValue>(ret);
}

uint32_t Assignment::level(literal_t lit) const {
    uint32_t ret;
    handleCError(clingo_assignment_level(ass_, lit, &ret));
    return ret;
}

literal_t Assignment::decision(uint32_t level) const {
    literal_t ret;
    handleCError(clingo_assignment_decision(ass_, level, &ret));
    return ret;
}

bool Assignment::is_fixed(literal_t lit) const {
    bool ret;
    handleCError(clingo_assignment_is_fixed(ass_, lit, &ret));
    return ret;
}

bool Assignment::is_true(literal_t lit) const {
    bool ret;
    handleCError(clingo_assignment_is_true(ass_, lit, &ret));
    return ret;
}

bool Assignment::is_false(literal_t lit) const {
    bool ret;
    handleCError(clingo_assignment_is_false(ass_, lit, &ret));
    return ret;
}

// {{{1 propagate init

literal_t PropagateInit::solver_literal(literal_t lit) const {
    literal_t ret;
    handleCError(clingo_propagate_init_solver_literal(init_, lit, &ret));
    return ret;
}

void PropagateInit::add_watch(literal_t lit) {
    handleCError(clingo_propagate_init_add_watch(init_, lit));
}

int PropagateInit::number_of_threads() const {
    return clingo_propagate_init_number_of_threads(init_);
}

SymbolicAtoms PropagateInit::symbolic_atoms() const {
    clingo_symbolic_atoms_t *ret;
    handleCError(clingo_propagate_init_symbolic_atoms(init_, &ret));
    return SymbolicAtoms{ret};
}

TheoryAtoms PropagateInit::theory_atoms() const {
    clingo_theory_atoms_t *ret;
    handleCError(clingo_propagate_init_theory_atoms(init_, &ret));
    return TheoryAtoms{ret};
}

// {{{1 propagate control

id_t PropagateControl::thread_id() const {
    return clingo_propagate_control_thread_id(ctl_);
}

Assignment PropagateControl::assignment() const {
    return Assignment{clingo_propagate_control_assignment(ctl_)};
}

literal_t PropagateControl::add_literal() {
    clingo_literal_t ret;
    handleCError(clingo_propagate_control_add_literal(ctl_, &ret));
    return ret;
}

void PropagateControl::add_watch(literal_t literal) {
    handleCError(clingo_propagate_control_add_watch(ctl_, literal));
}

bool PropagateControl::has_watch(literal_t literal) const {
    return clingo_propagate_control_has_watch(ctl_, literal);
}

void PropagateControl::remove_watch(literal_t literal) {
    clingo_propagate_control_remove_watch(ctl_, literal);
}

bool PropagateControl::add_clause(LiteralSpan clause, ClauseType type) {
    bool ret;
    handleCError(clingo_propagate_control_add_clause(ctl_, clause.begin(), clause.size(), static_cast<clingo_clause_type_t>(type), &ret));
    return ret;
}

bool PropagateControl::propagate() {
    bool ret;
    handleCError(clingo_propagate_control_propagate(ctl_, &ret));
    return ret;
}

// {{{1 propagator

void Propagator::init(PropagateInit &) { }
void Propagator::propagate(PropagateControl &, LiteralSpan) { }
void Propagator::undo(PropagateControl const &, LiteralSpan) { }
void Propagator::check(PropagateControl &) { }

// {{{1 solve control

void SolveControl::add_clause(SymbolicLiteralSpan clause) {
    handleCError(clingo_solve_control_add_clause(ctl_, reinterpret_cast<clingo_symbolic_literal_t const *>(clause.begin()), clause.size()));
}

id_t SolveControl::thread_id() const {
    id_t ret;
    handleCError(clingo_solve_control_thread_id(ctl_, &ret));
    return ret;
}

// {{{1 model

Model::Model(clingo_model_t *model)
: model_(model) { }

bool Model::contains(Symbol atom) const {
    bool ret;
    handleCError(clingo_model_contains(model_, atom.to_c(), &ret));
    return ret;
}

CostVector Model::cost() const {
    CostVector ret;
    size_t n;
    handleCError(clingo_model_cost_size(model_, &n));
    ret.resize(n);
    handleCError(clingo_model_cost(model_, ret.data(), n));
    return ret;
}

SymbolVector Model::symbols(ShowType show) const {
    SymbolVector ret;
    size_t n;
    handleCError(clingo_model_symbols_size(model_, show, &n));
    ret.resize(n);
    handleCError(clingo_model_symbols(model_, show, reinterpret_cast<clingo_symbol_t *>(ret.data()), n));
    return ret;
}

uint64_t Model::number() const {
    uint64_t ret;
    handleCError(clingo_model_number(model_, &ret));
    return ret;
}

bool Model::optimality_proven() const {
    bool ret;
    handleCError(clingo_model_optimality_proven(model_, &ret));
    return ret;
}

SolveControl Model::context() const {
    clingo_solve_control_t *ret;
    handleCError(clingo_model_context(model_, &ret));
    return SolveControl{ret};
}

ModelType Model::type() const {
    clingo_model_type_t ret;
    handleCError(clingo_model_type(model_, &ret));
    return static_cast<ModelType>(ret);
}

// {{{1 solve iter

SolveIteratively::SolveIteratively()
: iter_(nullptr) { }

SolveIteratively::SolveIteratively(clingo_solve_iteratively_t *it)
: iter_(it) { }

SolveIteratively::SolveIteratively(SolveIteratively &&it)
: iter_(nullptr) { std::swap(iter_, it.iter_); }

SolveIteratively &SolveIteratively::operator=(SolveIteratively &&it) {
    std::swap(iter_, it.iter_);
    return *this;
}

Model SolveIteratively::next() {
    clingo_model_t *m = nullptr;
    if (iter_) { handleCError(clingo_solve_iteratively_next(iter_, &m)); }
    return Model{m};
}

SolveResult SolveIteratively::get() {
    clingo_solve_result_bitset_t ret = 0;
    if (iter_) { handleCError(clingo_solve_iteratively_get(iter_, &ret)); }
    return SolveResult{ret};
}

void SolveIteratively::close() {
    if (iter_) {
        clingo_solve_iteratively_close(iter_);
        iter_ = nullptr;
    }
}

// {{{1 solve async

void SolveAsync::cancel() {
    handleCError(clingo_solve_async_cancel(async_));
}

SolveResult SolveAsync::get() {
    clingo_solve_result_bitset_t ret;
    handleCError(clingo_solve_async_get(async_, &ret));
    return SolveResult{ret};
}

bool SolveAsync::wait(double timeout) {
    bool ret;
    handleCError(clingo_solve_async_wait(async_, timeout, &ret));
    return ret;
}

// {{{1 backend

void Backend::rule(bool choice, AtomSpan head, LiteralSpan body) {
    handleCError(clingo_backend_rule(backend_, choice, head.begin(), head.size(), body.begin(), body.size()));
}

void Backend::weight_rule(bool choice, AtomSpan head, weight_t lower, WeightedLiteralSpan body) {
    handleCError(clingo_backend_weight_rule(backend_, choice, head.begin(), head.size(), lower, reinterpret_cast<clingo_weighted_literal_t const *>(body.begin()), body.size()));
}

void Backend::minimize(weight_t prio, WeightedLiteralSpan body) {
    handleCError(clingo_backend_minimize(backend_, prio, reinterpret_cast<clingo_weighted_literal_t const *>(body.begin()), body.size()));
}

void Backend::project(AtomSpan atoms) {
    handleCError(clingo_backend_project(backend_, atoms.begin(), atoms.size()));
}

void Backend::external(atom_t atom, ExternalType type) {
    handleCError(clingo_backend_external(backend_, atom, static_cast<clingo_external_type_t>(type)));
}

void Backend::assume(LiteralSpan lits) {
    handleCError(clingo_backend_assume(backend_, lits.begin(), lits.size()));
}

void Backend::heuristic(atom_t atom, HeuristicType type, int bias, unsigned priority, LiteralSpan condition) {
    handleCError(clingo_backend_heuristic(backend_, atom, static_cast<clingo_heuristic_type_t>(type), bias, priority, condition.begin(), condition.size()));
}

void Backend::acyc_edge(int node_u, int node_v, LiteralSpan condition) {
    handleCError(clingo_backend_acyc_edge(backend_, node_u, node_v, condition.begin(), condition.size()));
}

atom_t Backend::add_atom() {
    clingo_atom_t ret;
    handleCError(clingo_backend_add_atom(backend_, &ret));
    return ret;
}

// {{{1 statistics

StatisticsType Statistics::type() const {
    clingo_statistics_type_t ret;
    handleCError(clingo_statistics_type(stats_, key_, &ret));
    return StatisticsType(ret);
}

size_t Statistics::size() const {
    size_t ret;
    handleCError(clingo_statistics_array_size(stats_, key_, &ret));
    return ret;
}

Statistics Statistics::operator[](size_t index) const {
    uint64_t ret;
    handleCError(clingo_statistics_array_at(stats_, key_, index, &ret));
    return Statistics{stats_, ret};
}

StatisticsArrayIterator Statistics::begin() const {
    return StatisticsArrayIterator{this, 0};
}

StatisticsArrayIterator Statistics::end() const {
    return StatisticsArrayIterator{this, size()};
}

Statistics Statistics::operator[](char const *name) const {
    uint64_t ret;
    handleCError(clingo_statistics_map_at(stats_, key_, name, &ret));
    return Statistics{stats_, ret};
}

StatisticsKeyRange Statistics::keys() const {
    size_t ret;
    handleCError(clingo_statistics_map_size(stats_, key_, &ret));
    return StatisticsKeyRange{ StatisticsKeyIterator{this, 0}, StatisticsKeyIterator{this, ret} };
}

double Statistics::value() const {
    double ret;
    handleCError(clingo_statistics_value_get(stats_, key_, &ret));
    return ret;
}

char const *Statistics::key_name(size_t index) const {
    char const *ret;
    handleCError(clingo_statistics_map_subkey_name(stats_, key_, index, &ret));
    return ret;
}

// {{{1 configuration

Configuration Configuration::operator[](size_t index) {
    unsigned ret;
    handleCError(clingo_configuration_array_at(conf_, key_, index, &ret));
    return Configuration{conf_, ret};
}

ConfigurationArrayIterator Configuration::begin() {
    return ConfigurationArrayIterator{this, 0};
}

ConfigurationArrayIterator Configuration::end() {
    return ConfigurationArrayIterator{this, size()};
}

size_t Configuration::size() const {
    size_t n;
    handleCError(clingo_configuration_array_size(conf_, key_, &n));
    return n;
}

bool Configuration::empty() const {
    return size() == 0;
}

Configuration Configuration::operator[](char const *name) {
    clingo_id_t ret;
    handleCError(clingo_configuration_map_at(conf_, key_, name, &ret));
    return Configuration{conf_, ret};
}

ConfigurationKeyRange Configuration::keys() const {
    size_t n;
    handleCError(clingo_configuration_map_size(conf_, key_, &n));
    return ConfigurationKeyRange{ ConfigurationKeyIterator{this, size_t(0)}, ConfigurationKeyIterator{this, size_t(n)} };
}

bool Configuration::is_value() const {
    clingo_configuration_type_bitset_t type;
    handleCError(clingo_configuration_type(conf_, key_, &type));
    return type & clingo_configuration_type_value;
}

bool Configuration::is_array() const {
    clingo_configuration_type_bitset_t type;
    handleCError(clingo_configuration_type(conf_, key_, &type));
    return (type & clingo_configuration_type_array) != 0;
}

bool Configuration::is_map() const {
    clingo_configuration_type_bitset_t type;
    handleCError(clingo_configuration_type(conf_, key_, &type));
    return (type & clingo_configuration_type_map) != 0;
}

bool Configuration::is_assigned() const {
    bool ret;
    handleCError(clingo_configuration_value_is_assigned(conf_, key_, &ret));
    return ret;
}

std::string Configuration::value() const {
    size_t n;
    handleCError(clingo_configuration_value_get_size(conf_, key_, &n));
    std::vector<char> ret(n);
    handleCError(clingo_configuration_value_get(conf_, key_, ret.data(), n));
    return std::string(ret.begin(), ret.end() - 1);
}

Configuration &Configuration::operator=(char const *value) {
    handleCError(clingo_configuration_value_set(conf_, key_, value));
    return *this;
}

char const *Configuration::decription() const {
    char const *ret;
    handleCError(clingo_configuration_description(conf_, key_, &ret));
    return ret;
}

char const *Configuration::key_name(size_t index) const {
    char const *ret;
    handleCError(clingo_configuration_map_subkey_name(conf_, key_, index, &ret));
    return ret;
}

// {{{1 program builder

namespace AST { namespace {

struct ASTToC {
    // {{{2 term

    clingo_ast_id_t convId(Id const &id) {
        return {id.location, id.id};
    }

    struct TermTag {};

    clingo_ast_term_t visit(Symbol const &x, TermTag) {
        clingo_ast_term_t ret;
        ret.type     = clingo_ast_term_type_symbol;
        ret.symbol   = x.to_c();
        return ret;
    }
    clingo_ast_term_t visit(Variable const &x, TermTag) {
        clingo_ast_term_t ret;
        ret.type     = clingo_ast_term_type_variable;
        ret.variable = x.name;
        return ret;
    }
    clingo_ast_term_t visit(UnaryOperation const &x, TermTag) {
        auto unary_operation = create_<clingo_ast_unary_operation_t>();
        unary_operation->unary_operator = static_cast<clingo_ast_unary_operator_t>(x.unary_operator);
        unary_operation->argument       = convTerm(x.argument);
        clingo_ast_term_t ret;
        ret.type            = clingo_ast_term_type_unary_operation;
        ret.unary_operation = unary_operation;
        return ret;
    }
    clingo_ast_term_t visit(BinaryOperation const &x, TermTag) {
        auto binary_operation = create_<clingo_ast_binary_operation_t>();
        binary_operation->binary_operator = static_cast<clingo_ast_binary_operator_t>(x.binary_operator);
        binary_operation->left            = convTerm(x.left);
        binary_operation->right           = convTerm(x.right);
        clingo_ast_term_t ret;
        ret.type             = clingo_ast_term_type_binary_operation;
        ret.binary_operation = binary_operation;
        return ret;
    }
    clingo_ast_term_t visit(Interval const &x, TermTag) {
        auto interval = create_<clingo_ast_interval_t>();
        interval->left  = convTerm(x.left);
        interval->right = convTerm(x.right);
        clingo_ast_term_t ret;
        ret.type     = clingo_ast_term_type_interval;
        ret.interval = interval;
        return ret;
    }
    clingo_ast_term_t visit(Function const &x, TermTag) {
        auto function = create_<clingo_ast_function_t>();
        function->name      = x.name;
        function->arguments = convTermVec(x.arguments);
        function->size      = x.arguments.size();
        clingo_ast_term_t ret;
        ret.type     = x.external ? clingo_ast_term_type_external_function : clingo_ast_term_type_function;
        ret.function = function;
        return ret;
    }
    clingo_ast_term_t visit(Pool const &x, TermTag) {
        auto pool = create_<clingo_ast_pool_t>();
        pool->arguments = convTermVec(x.arguments);
        pool->size      = x.arguments.size();
        clingo_ast_term_t ret;
        ret.type     = clingo_ast_term_type_pool;
        ret.pool     = pool;
        return ret;
    }
    clingo_ast_term_t convTerm(Term const &x) {
        auto ret = x.data.accept(*this, TermTag{});
        ret.location = x.location;
        return ret;
    }
    clingo_ast_term_t *convTerm(Optional<Term> const &x) {
        return x ? create_(convTerm(*x.get())) : nullptr;
    }
    clingo_ast_term_t *convTermVec(std::vector<Term> const &x) {
        return createArray_(x, static_cast<clingo_ast_term_t (ASTToC::*)(Term const &)>(&ASTToC::convTerm));
    }

    clingo_ast_csp_product_term_t convCSPProduct(CSPProduct const &x) {
        clingo_ast_csp_product_term_t ret;
        ret.location    = x.location;
        ret.variable    = convTerm(x.variable);
        ret.coefficient = convTerm(x.coefficient);
        return ret;
    }
    clingo_ast_csp_sum_term_t convCSPAdd(CSPSum const &x) {
        clingo_ast_csp_sum_term_t ret;
        ret.location = x.location;
        ret.terms    = createArray_(x.terms, &ASTToC::convCSPProduct);
        ret.size     = x.terms.size();
        return ret;
    }

    clingo_ast_theory_unparsed_term_element_t convTheoryUnparsedTermElement(TheoryUnparsedTermElement const &x) {
        clingo_ast_theory_unparsed_term_element_t ret;
        ret.term      = convTheoryTerm(x.term);
        ret.operators = createArray_(x.operators, &ASTToC::identity<char const *>);
        ret.size      = x.operators.size();
        return ret;
    }

    struct TheoryTermTag { };

    clingo_ast_theory_term_t visit(Symbol const &term, TheoryTermTag) {
        clingo_ast_theory_term_t ret;
        ret.type     = clingo_ast_theory_term_type_symbol;
        ret.symbol   = term.to_c();
        return ret;
    }
    clingo_ast_theory_term_t visit(Variable const &term, TheoryTermTag) {
        clingo_ast_theory_term_t ret;
        ret.type     = clingo_ast_theory_term_type_variable;
        ret.variable = term.name;
        return ret;
    }
    clingo_ast_theory_term_t visit(TheoryTermSequence const &term, TheoryTermTag) {
        auto sequence = create_<clingo_ast_theory_term_array_t>();
        sequence->terms = convTheoryTermVec(term.terms);
        sequence->size  = term.terms.size();
        clingo_ast_theory_term_t ret;
        switch (term.type) {
            case TheoryTermSequenceType::Set:   { ret.type = clingo_ast_theory_term_type_set; break; }
            case TheoryTermSequenceType::List:  { ret.type = clingo_ast_theory_term_type_list; break; }
            case TheoryTermSequenceType::Tuple: { ret.type = clingo_ast_theory_term_type_tuple; break; }
        }
        ret.set = sequence;
        return ret;
    }
    clingo_ast_theory_term_t visit(TheoryFunction const &term, TheoryTermTag) {
        auto function = create_<clingo_ast_theory_function_t>();
        function->name      = term.name;
        function->arguments = convTheoryTermVec(term.arguments);
        function->size      = term.arguments.size();
        clingo_ast_theory_term_t ret;
        ret.type     = clingo_ast_theory_term_type_function;
        ret.function = function;
        return ret;
    }
    clingo_ast_theory_term_t visit(TheoryUnparsedTerm const &term, TheoryTermTag) {
        auto unparsed_term = create_<clingo_ast_theory_unparsed_term>();
        unparsed_term->elements = createArray_(term.elements, &ASTToC::convTheoryUnparsedTermElement);
        unparsed_term->size     = term.elements.size();
        clingo_ast_theory_term_t ret;
        ret.type          = clingo_ast_theory_term_type_unparsed_term;
        ret.unparsed_term = unparsed_term;
        return ret;
    }
    clingo_ast_theory_term_t convTheoryTerm(TheoryTerm const &x) {
        auto ret = x.data.accept(*this, TheoryTermTag{});
        ret.location = x.location;
        return ret;
    }
    clingo_ast_theory_term_t *convTheoryTermVec(std::vector<TheoryTerm> const &x) {
        return createArray_(x, &ASTToC::convTheoryTerm);
    }

    // {{{2 literal

    clingo_ast_csp_guard_t convCSPGuard(CSPGuard const &x) {
        clingo_ast_csp_guard_t ret;
        ret.comparison = static_cast<clingo_ast_comparison_operator_t>(x.comparison);
        ret.term       = convCSPAdd(x.term);
        return ret;
    }

    clingo_ast_literal_t visit(Boolean const &x) {
        clingo_ast_literal_t ret;
        ret.type     = clingo_ast_literal_type_boolean;
        ret.boolean  = x.value;
        return ret;
    }
    clingo_ast_literal_t visit(Term const &x) {
        clingo_ast_literal_t ret;
        ret.type     = clingo_ast_literal_type_symbolic;
        ret.symbol   = create_<clingo_ast_term_t>(convTerm(x));
        return ret;
    }
    clingo_ast_literal_t visit(Comparison const &x) {
        auto comparison = create_<clingo_ast_comparison_t>();
        comparison->comparison = static_cast<clingo_ast_comparison_operator_t>(x.comparison);
        comparison->left       = convTerm(x.left);
        comparison->right      = convTerm(x.right);
        clingo_ast_literal_t ret;
        ret.type       = clingo_ast_literal_type_comparison;
        ret.comparison = comparison;
        return ret;
    }
    clingo_ast_literal_t visit(CSPLiteral const &x) {
        auto csp = create_<clingo_ast_csp_literal_t>();
        csp->term   = convCSPAdd(x.term);
        csp->guards = createArray_(x.guards, &ASTToC::convCSPGuard);
        csp->size   = x.guards.size();
        clingo_ast_literal_t ret;
        ret.type        = clingo_ast_literal_type_csp;
        ret.csp_literal = csp;
        return ret;
    }
    clingo_ast_literal_t convLiteral(Literal const &x) {
        auto ret = x.data.accept(*this);
        ret.sign     = static_cast<clingo_ast_sign_t>(x.sign);
        ret.location = x.location;
        return ret;
    }
    clingo_ast_literal_t *convLiteralVec(std::vector<Literal> const &x) {
        return createArray_(x, &ASTToC::convLiteral);
    }

    // {{{2 aggregates

    clingo_ast_aggregate_guard_t *convAggregateGuard(Optional<AggregateGuard> const &guard) {
        return guard
            ? create_<clingo_ast_aggregate_guard_t>({static_cast<clingo_ast_comparison_operator_t>(guard->comparison), convTerm(guard->term)})
            : nullptr;
    }

    clingo_ast_conditional_literal_t convConditionalLiteral(ConditionalLiteral const &x) {
        clingo_ast_conditional_literal_t ret;
        ret.literal   = convLiteral(x.literal);
        ret.condition = convLiteralVec(x.condition);
        ret.size      = x.condition.size();
        return ret;
    }

    clingo_ast_theory_guard_t *convTheoryGuard(Optional<TheoryGuard> const &x) {
        return x
            ? create_<clingo_ast_theory_guard_t>({x->operator_name, convTheoryTerm(x->term)})
            : nullptr;
    }

    clingo_ast_theory_atom_element_t convTheoryAtomElement(TheoryAtomElement const &x) {
        clingo_ast_theory_atom_element_t ret;
        ret.tuple          = convTheoryTermVec(x.tuple);
        ret.tuple_size     = x.tuple.size();
        ret.condition      = convLiteralVec(x.condition);
        ret.condition_size = x.condition.size();
        return ret;
    }

    clingo_ast_body_aggregate_element_t convBodyAggregateElement(BodyAggregateElement const &x) {
        clingo_ast_body_aggregate_element_t ret;
        ret.tuple          = convTermVec(x.tuple);
        ret.tuple_size     = x.tuple.size();
        ret.condition      = convLiteralVec(x.condition);
        ret.condition_size = x.condition.size();
        return ret;
    }

    clingo_ast_head_aggregate_element_t convHeadAggregateElement(HeadAggregateElement const &x) {
        clingo_ast_head_aggregate_element_t ret;
        ret.tuple               = convTermVec(x.tuple);
        ret.tuple_size          = x.tuple.size();
        ret.conditional_literal = convConditionalLiteral(x.condition);
        return ret;
    }

    clingo_ast_aggregate_t convAggregate(Aggregate const &x) {
        clingo_ast_aggregate_t ret;
        ret.left_guard  = convAggregateGuard(x.left_guard);
        ret.right_guard = convAggregateGuard(x.right_guard);
        ret.size        = x.elements.size();
        ret.elements    = createArray_(x.elements, &ASTToC::convConditionalLiteral);
        return ret;
    }

    clingo_ast_theory_atom_t convTheoryAtom(TheoryAtom const &x) {
        clingo_ast_theory_atom_t ret;
        ret.term     = convTerm(x.term);
        ret.guard    = convTheoryGuard(x.guard);
        ret.elements = createArray_(x.elements, &ASTToC::convTheoryAtomElement);
        ret.size     = x.elements.size();
        return ret;
    }

    clingo_ast_disjoint_element_t convDisjointElement(DisjointElement const &x) {
        clingo_ast_disjoint_element_t ret;
        ret.location       = x.location;
        ret.tuple          = convTermVec(x.tuple);
        ret.tuple_size     = x.tuple.size();
        ret.term           = convCSPAdd(x.term);
        ret.condition      = convLiteralVec(x.condition);
        ret.condition_size = x.condition.size();
        return ret;
    }

    // {{{2 head literal

    struct HeadLiteralTag { };

    clingo_ast_head_literal_t visit(Literal const &x, HeadLiteralTag) {
        clingo_ast_head_literal_t ret;
        ret.type     = clingo_ast_head_literal_type_literal;
        ret.literal  = create_<clingo_ast_literal_t>(convLiteral(x));
        return ret;
    }
    clingo_ast_head_literal_t visit(Disjunction const &x, HeadLiteralTag) {
        auto disjunction = create_<clingo_ast_disjunction_t>();
        disjunction->size     = x.elements.size();
        disjunction->elements = createArray_(x.elements, &ASTToC::convConditionalLiteral);
        clingo_ast_head_literal_t ret;
        ret.type        = clingo_ast_head_literal_type_disjunction;
        ret.disjunction = disjunction;
        return ret;
    }
    clingo_ast_head_literal_t visit(Aggregate const &x, HeadLiteralTag) {
        clingo_ast_head_literal_t ret;
        ret.type      = clingo_ast_head_literal_type_aggregate;
        ret.aggregate = create_<clingo_ast_aggregate_t>(convAggregate(x));
        return ret;
    }
    clingo_ast_head_literal_t visit(HeadAggregate const &x, HeadLiteralTag) {
        auto head_aggregate = create_<clingo_ast_head_aggregate_t>();
        head_aggregate->left_guard  = convAggregateGuard(x.left_guard);
        head_aggregate->right_guard = convAggregateGuard(x.right_guard);
        head_aggregate->function    = static_cast<clingo_ast_aggregate_function_t>(x.function);
        head_aggregate->size        = x.elements.size();
        head_aggregate->elements    = createArray_(x.elements, &ASTToC::convHeadAggregateElement);
        clingo_ast_head_literal_t ret;
        ret.type           = clingo_ast_head_literal_type_head_aggregate;
        ret.head_aggregate = head_aggregate;
        return ret;
    }
    clingo_ast_head_literal_t visit(TheoryAtom const &x, HeadLiteralTag) {
        clingo_ast_head_literal_t ret;
        ret.type        = clingo_ast_head_literal_type_theory_atom;
        ret.theory_atom = create_<clingo_ast_theory_atom_t>(convTheoryAtom(x));
        return ret;
    }
    clingo_ast_head_literal_t convHeadLiteral(HeadLiteral const &lit) {
        auto ret = lit.data.accept(*this, HeadLiteralTag{});
        ret.location = lit.location;
        return ret;
    }

    // {{{2 body literal

    struct BodyLiteralTag { };

    clingo_ast_body_literal_t visit(Literal const &x, BodyLiteralTag) {
        clingo_ast_body_literal_t ret;
        ret.type     = clingo_ast_body_literal_type_literal;
        ret.literal  = create_<clingo_ast_literal_t>(convLiteral(x));
        return ret;
    }
    clingo_ast_body_literal_t visit(ConditionalLiteral const &x, BodyLiteralTag) {
        clingo_ast_body_literal_t ret;
        ret.type        = clingo_ast_body_literal_type_conditional;
        ret.conditional = create_<clingo_ast_conditional_literal_t>(convConditionalLiteral(x));
        return ret;
    }
    clingo_ast_body_literal_t visit(Aggregate const &x, BodyLiteralTag) {
        clingo_ast_body_literal_t ret;
        ret.type      = clingo_ast_body_literal_type_aggregate;
        ret.aggregate = create_<clingo_ast_aggregate_t>(convAggregate(x));
        return ret;
    }
    clingo_ast_body_literal_t visit(BodyAggregate const &x, BodyLiteralTag) {
        auto body_aggregate = create_<clingo_ast_body_aggregate_t>();
        body_aggregate->left_guard  = convAggregateGuard(x.left_guard);
        body_aggregate->right_guard = convAggregateGuard(x.right_guard);
        body_aggregate->function    = static_cast<clingo_ast_aggregate_function_t>(x.function);
        body_aggregate->size        = x.elements.size();
        body_aggregate->elements    = createArray_(x.elements, &ASTToC::convBodyAggregateElement);
        clingo_ast_body_literal_t ret;
        ret.type     = clingo_ast_body_literal_type_body_aggregate;
        ret.body_aggregate = body_aggregate;
        return ret;
    }
    clingo_ast_body_literal_t visit(TheoryAtom const &x, BodyLiteralTag) {
        clingo_ast_body_literal_t ret;
        ret.type        = clingo_ast_body_literal_type_theory_atom;
        ret.theory_atom = create_<clingo_ast_theory_atom_t>(convTheoryAtom(x));
        return ret;
    }
    clingo_ast_body_literal_t visit(Disjoint const &x, BodyLiteralTag) {
        auto disjoint = create_<clingo_ast_disjoint_t>();
        disjoint->size     = x.elements.size();
        disjoint->elements = createArray_(x.elements, &ASTToC::convDisjointElement);
        clingo_ast_body_literal_t ret;
        ret.type     = clingo_ast_body_literal_type_disjoint;
        ret.disjoint = disjoint;
        return ret;
    }

    clingo_ast_body_literal_t convBodyLiteral(BodyLiteral const &x) {
        auto ret = x.data.accept(*this, BodyLiteralTag{});
        ret.sign     = static_cast<clingo_ast_sign_t>(x.sign);
        ret.location = x.location;
        return ret;
    }

    clingo_ast_body_literal_t* convBodyLiteralVec(std::vector<BodyLiteral> const &x) {
        return createArray_(x, &ASTToC::convBodyLiteral);
    }

    // {{{2 theory definitions

    clingo_ast_theory_operator_definition_t convTheoryOperatorDefinition(TheoryOperatorDefinition const &x) {
        clingo_ast_theory_operator_definition_t ret;
        ret.type     = static_cast<clingo_ast_theory_operator_type_t>(x.type);
        ret.priority = x.priority;
        ret.location = x.location;
        ret.name     = x.name;
        return ret;
    }

    clingo_ast_theory_term_definition_t convTheoryTermDefinition(TheoryTermDefinition const &x) {
        clingo_ast_theory_term_definition_t ret;
        ret.name      = x.name;
        ret.location  = x.location;
        ret.operators = createArray_(x.operators, &ASTToC::convTheoryOperatorDefinition);
        ret.size      = x.operators.size();
        return ret;
    }

    clingo_ast_theory_guard_definition_t convTheoryGuardDefinition(TheoryGuardDefinition const &x) {
        clingo_ast_theory_guard_definition_t ret;
        ret.term      = x.term;
        ret.operators = createArray_(x.operators, &ASTToC::identity<char const *>);
        ret.size      = x.operators.size();
        return ret;
    }

    clingo_ast_theory_atom_definition_t convTheoryAtomDefinition(TheoryAtomDefinition const &x) {
        clingo_ast_theory_atom_definition_t ret;
        ret.name     = x.name;
        ret.arity    = x.arity;
        ret.location = x.location;
        ret.type     = static_cast<clingo_ast_theory_atom_definition_type_t>(x.type);
        ret.elements = x.elements;
        ret.guard    = x.guard ? create_<clingo_ast_theory_guard_definition_t>(convTheoryGuardDefinition(*x.guard.get())) : nullptr;
        return ret;
    }

    // {{{2 statement

    clingo_ast_statement_t visit(Rule const &x) {
        auto *rule = create_<clingo_ast_rule_t>();
        rule->head = convHeadLiteral(x.head);
        rule->size = x.body.size();
        rule->body = convBodyLiteralVec(x.body);
        clingo_ast_statement_t ret;
        ret.type     = clingo_ast_statement_type_rule;
        ret.rule     = rule;
        return ret;
    }
    clingo_ast_statement_t visit(Definition const &x) {
        auto *definition = create_<clingo_ast_definition_t>();
        definition->is_default = x.is_default;
        definition->name       = x.name;
        definition->value      = convTerm(x.value);
        clingo_ast_statement_t ret;
        ret.type       = clingo_ast_statement_type_const;
        ret.definition = definition;
        return ret;
    }
    clingo_ast_statement_t visit(ShowSignature const &x) {
        auto *show_signature = create_<clingo_ast_show_signature_t>();
        show_signature->csp       = x.csp;
        show_signature->signature = x.signature.to_c();
        clingo_ast_statement_t ret;
        ret.type           = clingo_ast_statement_type_show_signature;
        ret.show_signature = show_signature;
        return ret;
    }
    clingo_ast_statement_t visit(ShowTerm const &x) {
        auto *show_term = create_<clingo_ast_show_term_t>();
        show_term->csp  = x.csp;
        show_term->term = convTerm(x.term);
        show_term->body = convBodyLiteralVec(x.body);
        show_term->size = x.body.size();
        clingo_ast_statement_t ret;
        ret.type      = clingo_ast_statement_type_show_term;
        ret.show_term = show_term;
        return ret;
    }
    clingo_ast_statement_t visit(Minimize const &x) {
        auto *minimize = create_<clingo_ast_minimize_t>();
        minimize->weight     = convTerm(x.weight);
        minimize->priority   = convTerm(x.priority);
        minimize->tuple      = convTermVec(x.tuple);
        minimize->tuple_size = x.tuple.size();
        minimize->body       = convBodyLiteralVec(x.body);
        minimize->body_size  = x.body.size();
        clingo_ast_statement_t ret;
        ret.type     = clingo_ast_statement_type_minimize;
        ret.minimize = minimize;
        return ret;
    }
    clingo_ast_statement_t visit(Script const &x) {
        auto *script = create_<clingo_ast_script_t>();
        script->type = static_cast<clingo_ast_script_type_t>(x.type);
        script->code = x.code;
        clingo_ast_statement_t ret;
        ret.type   = clingo_ast_statement_type_script;
        ret.script = script;
        return ret;
    }
    clingo_ast_statement_t visit(Program const &x) {
        auto *program = create_<clingo_ast_program_t>();
        program->name       = x.name;
        program->parameters = createArray_(x.parameters, &ASTToC::convId);
        program->size       = x.parameters.size();
        clingo_ast_statement_t ret;
        ret.type    = clingo_ast_statement_type_program;
        ret.program = program;
        return ret;
    }
    clingo_ast_statement_t visit(External const &x) {
        auto *external = create_<clingo_ast_external_t>();
        external->atom = convTerm(x.atom);
        external->body = convBodyLiteralVec(x.body);
        external->size = x.body.size();
        clingo_ast_statement_t ret;
        ret.type     = clingo_ast_statement_type_external;
        ret.external = external;
        return ret;
    }
    clingo_ast_statement_t visit(Edge const &x) {
        auto *edge = create_<clingo_ast_edge_t>();
        edge->u    = convTerm(x.u);
        edge->v    = convTerm(x.v);
        edge->body = convBodyLiteralVec(x.body);
        edge->size = x.body.size();
        clingo_ast_statement_t ret;
        ret.type = clingo_ast_statement_type_edge;
        ret.edge = edge;
        return ret;
    }
    clingo_ast_statement_t visit(Heuristic const &x) {
        auto *heuristic = create_<clingo_ast_heuristic_t>();
        heuristic->atom     = convTerm(x.atom);
        heuristic->bias     = convTerm(x.bias);
        heuristic->priority = convTerm(x.priority);
        heuristic->modifier = convTerm(x.modifier);
        heuristic->body     = convBodyLiteralVec(x.body);
        heuristic->size     = x.body.size();
        clingo_ast_statement_t ret;
        ret.type      = clingo_ast_statement_type_heuristic;
        ret.heuristic = heuristic;
        return ret;
    }
    clingo_ast_statement_t visit(ProjectAtom const &x) {
        auto *project = create_<clingo_ast_project_t>();
        project->atom = convTerm(x.atom);
        project->body = convBodyLiteralVec(x.body);
        project->size = x.body.size();
        clingo_ast_statement_t ret;
        ret.type         = clingo_ast_statement_type_project_atom;
        ret.project_atom = project;
        return ret;
    }
    clingo_ast_statement_t visit(ProjectSignature const &x) {
        clingo_ast_statement_t ret;
        ret.type              = clingo_ast_statement_type_project_atom_signature;
        ret.project_signature = x.signature.to_c();
        return ret;
    }
    clingo_ast_statement_t visit(TheoryDefinition const &x) {
        auto *theory_definition = create_<clingo_ast_theory_definition_t>();
        theory_definition->name       = x.name;
theory_definition->terms = createArray_(x.terms, &ASTToC::convTheoryTermDefinition);
theory_definition->terms_size = x.terms.size();
theory_definition->atoms = createArray_(x.atoms, &ASTToC::convTheoryAtomDefinition);
theory_definition->atoms_size = x.atoms.size();
clingo_ast_statement_t ret;
ret.type = clingo_ast_statement_type_theory_definition;
ret.theory_definition = theory_definition;
return ret;
    }

    // {{{2 aux

    template <class T>
    T identity(T t) { return t; }

    template <class T>
    T *create_() {
        data_.emplace_back(operator new(sizeof(T)));
        return reinterpret_cast<T*>(data_.back());
    }
    template <class T>
    T *create_(T x) {
        auto *r = create_<T>();
        *r = x;
        return r;
    }
    template <class T>
    T *createArray_(size_t size) {
        arrdata_.emplace_back(operator new[](sizeof(T) * size));
        return reinterpret_cast<T*>(arrdata_.back());
    }
    template <class T, class F>
    auto createArray_(std::vector<T> const &vec, F f) -> decltype((this->*f)(std::declval<T>()))* {
        using U = decltype((this->*f)(std::declval<T>()));
        auto r = createArray_<U>(vec.size()), jt = r;
        for (auto it = vec.begin(), ie = vec.end(); it != ie; ++it, ++jt) { *jt = (this->*f)(*it); }
        return r;
    }

    ~ASTToC() noexcept {
        for (auto &x : data_) { operator delete(x); }
        for (auto &x : arrdata_) { operator delete[](x); }
        data_.clear();
        arrdata_.clear();
    }

    std::vector<void *> data_;
    std::vector<void *> arrdata_;

    // }}}2
};

} } // namespace AST

void ProgramBuilder::begin() {
    handleCError(clingo_program_builder_begin(builder_));
}

void ProgramBuilder::add(AST::Statement const &stm) {
    AST::ASTToC a;
    auto x = stm.data.accept(a);
    x.location = stm.location;
    handleCError(clingo_program_builder_add(builder_, &x));
}

void ProgramBuilder::end() {
    handleCError(clingo_program_builder_end(builder_));
}

// {{{1 control

struct Control::Impl {
    Impl(Logger logger)
        : ctl(nullptr)
        , logger(logger) { }
    Impl(clingo_control_t *ctl)
        : ctl(ctl) { }
    ~Impl() noexcept {
        if (ctl) { clingo_control_free(ctl); }
    }
    operator clingo_control_t *() { return ctl; }
    clingo_control_t *ctl;
    Logger logger;
    ModelCallback mh;
    FinishCallback fh;
};

Control::Control(clingo_control_t *ctl)
    : impl_(new Impl(ctl)) { }

Control::Control(Control &&c)
    : impl_(nullptr) {
    std::swap(impl_, c.impl_);
}

Control &Control::operator=(Control &&c) {
    delete impl_;
    impl_ = nullptr;
    std::swap(impl_, c.impl_);
    return *this;
}

Control::~Control() noexcept {
    delete impl_;
}

void Control::add(char const *name, StringSpan params, char const *part) {
    handleCError(clingo_control_add(*impl_, name, params.begin(), params.size(), part));
}

void Control::ground(PartSpan parts, GroundCallback cb) {
    using Data = std::pair<GroundCallback&, std::exception_ptr>;
    Data data(cb, nullptr);
    handleCError(clingo_control_ground(*impl_, reinterpret_cast<clingo_part_t const *>(parts.begin()), parts.size(),
        [](clingo_location_t loc, char const *name, clingo_symbol_t const *args, size_t n, void *data, clingo_symbol_callback_t *cb, void *cbdata) -> bool {
            auto &d = *static_cast<Data*>(data);
            CLINGO_CALLBACK_TRY {
                if (d.first) {
                    struct Ret : std::exception { };
                    try {
                        d.first(Location(loc), name, {reinterpret_cast<Symbol const *>(args), n}, [cb, cbdata](SymbolSpan symret) {
                            if (!cb(reinterpret_cast<clingo_symbol_t const *>(symret.begin()), symret.size(), cbdata)) { throw Ret(); }
                        });
                    }
                    catch (Ret e) { return false; }
                }
            }
            CLINGO_CALLBACK_CATCH(d.second);
        }, &data), &data.second);
}

clingo_control_t *Control::to_c() const { return *impl_; }

SolveResult Control::solve(ModelCallback mh, SymbolicLiteralSpan assumptions) {
    clingo_solve_result_bitset_t ret;
    using Data = std::pair<ModelCallback&, std::exception_ptr>;
    Data data(mh, nullptr);
    handleCError(clingo_control_solve(*impl_, [](clingo_model_t *m, void *data, bool *ret) -> bool {
        auto &d = *static_cast<Data*>(data);
        CLINGO_CALLBACK_TRY { *ret = !d.first || d.first(Model(m)); }
        CLINGO_CALLBACK_CATCH(d.second);
    }, &data, reinterpret_cast<clingo_symbolic_literal_t const *>(assumptions.begin()), assumptions.size(), &ret));
    return SolveResult{ret};
}

SolveIteratively Control::solve_iteratively(SymbolicLiteralSpan assumptions) {
    clingo_solve_iteratively_t *it;
    handleCError(clingo_control_solve_iteratively(*impl_, reinterpret_cast<clingo_symbolic_literal_t const *>(assumptions.begin()), assumptions.size(), &it));
    return SolveIteratively{it};
}

void Control::assign_external(Symbol atom, TruthValue value) {
    handleCError(clingo_control_assign_external(*impl_, atom.to_c(), static_cast<clingo_truth_value_t>(value)));
}

void Control::release_external(Symbol atom) {
    handleCError(clingo_control_release_external(*impl_, atom.to_c()));
}

SymbolicAtoms Control::symbolic_atoms() const {
    clingo_symbolic_atoms_t *ret;
    handleCError(clingo_control_symbolic_atoms(*impl_, &ret));
    return SymbolicAtoms{ret};
}

TheoryAtoms Control::theory_atoms() const {
    clingo_theory_atoms_t *ret;
    clingo_control_theory_atoms(*impl_, &ret);
    return TheoryAtoms{ret};
}

namespace {

// NOTE: this sets exceptions in the running (propagation) thread(s)
// CAVEATS:
//   exceptions during propagation are not rethrown
//   they are thrown as new exceptions (with the same error message if available)
//   clasp gobbles exceptions in the multithreaded case
static bool g_init(clingo_propagate_init_t *ctl, Propagator *p) {
    GRINGO_CLINGO_TRY {
        PropagateInit pi(ctl);
        p->init(pi);
    }
    GRINGO_CLINGO_CATCH;
}

static bool g_propagate(clingo_propagate_control_t *ctl, clingo_literal_t const *changes, size_t n, Propagator *p) {
    GRINGO_CLINGO_TRY {
        PropagateControl pc(ctl);
        p->propagate(pc, {changes, n});
    }
    GRINGO_CLINGO_CATCH;
}

static bool g_undo(clingo_propagate_control_t *ctl, clingo_literal_t const *changes, size_t n, Propagator *p) {
    GRINGO_CLINGO_TRY {
        PropagateControl pc(ctl);
        p->undo(pc, {changes, n});
    }
    GRINGO_CLINGO_CATCH;
}

static bool g_check(clingo_propagate_control_t *ctl, Propagator *p) {
    GRINGO_CLINGO_TRY {
        PropagateControl pc(ctl);
        p->check(pc);
    }
    GRINGO_CLINGO_CATCH;
}

static clingo_propagator_t g_propagator = {
    reinterpret_cast<decltype(clingo_propagator_t::init)>(g_init),
    reinterpret_cast<decltype(clingo_propagator_t::propagate)>(g_propagate),
    reinterpret_cast<decltype(clingo_propagator_t::undo)>(g_undo),
    reinterpret_cast<decltype(clingo_propagator_t::check)>(g_check)
};

}

void Control::register_propagator(Propagator &propagator, bool sequential) {
    handleCError(clingo_control_register_propagator(*impl_, g_propagator, &propagator, sequential));
}

namespace {

bool g_init_program(bool incremental, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->init_program(incremental); }
    GRINGO_CLINGO_CATCH;
}
bool g_begin_step(GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->begin_step(); }
    GRINGO_CLINGO_CATCH;
}
bool g_end_step(GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->end_step(); }
    GRINGO_CLINGO_CATCH;
}

bool g_rule(bool choice, clingo_atom_t const *head, size_t head_size, clingo_literal_t const *body, size_t body_size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->rule(choice, AtomSpan(head, head_size), LiteralSpan(body, body_size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_weight_rule(bool choice, clingo_atom_t const *head, size_t head_size, clingo_weight_t lower_bound, clingo_weighted_literal_t const *body, size_t body_size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->weight_rule(choice, AtomSpan(head, head_size), lower_bound, WeightedLiteralSpan(reinterpret_cast<WeightedLiteral const*>(body), body_size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_minimize(clingo_weight_t priority, clingo_weighted_literal_t const* literals, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->minimize(priority, WeightedLiteralSpan(reinterpret_cast<WeightedLiteral const*>(literals), size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_project(clingo_atom_t const *atoms, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->project(AtomSpan(atoms, size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_output_atom(clingo_symbol_t symbol, clingo_atom_t atom, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->output_atom(Symbol{symbol}, atom); }
    GRINGO_CLINGO_CATCH;
}
bool g_output_term(clingo_symbol_t symbol, clingo_literal_t const *condition, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->output_term(Symbol{symbol}, LiteralSpan{condition, size}); }
    GRINGO_CLINGO_CATCH;
}
bool g_output_csp(clingo_symbol_t symbol, int value, clingo_literal_t const *condition, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->output_csp(Symbol{symbol}, value, LiteralSpan{condition, size}); }
    GRINGO_CLINGO_CATCH;
}
bool g_external(clingo_atom_t atom, clingo_external_type_t type, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->external(atom, static_cast<ExternalType>(type)); }
    GRINGO_CLINGO_CATCH;
}
bool g_assume(clingo_literal_t const *literals, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->assume(LiteralSpan(literals, size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_heuristic(clingo_atom_t atom, clingo_heuristic_type_t type, int bias, unsigned priority, clingo_literal_t const *condition, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->heuristic(atom, static_cast<HeuristicType>(type), bias, priority, LiteralSpan(condition, size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_acyc_edge(int node_u, int node_v, clingo_literal_t const *condition, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->acyc_edge(node_u, node_v, LiteralSpan(condition, size)); }
    GRINGO_CLINGO_CATCH;
}

bool g_theory_term_number(clingo_id_t term_id, int number, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->theory_term_number(term_id, number); }
    GRINGO_CLINGO_CATCH;
}
bool g_theory_term_string(clingo_id_t term_id, char const *name, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->theory_term_string(term_id, name); }
    GRINGO_CLINGO_CATCH;
}
bool g_theory_term_compound(clingo_id_t term_id, int name_id_or_type, clingo_id_t const *arguments, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->theory_term_compound(term_id, name_id_or_type, IdSpan(arguments, size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_theory_element(clingo_id_t element_id, clingo_id_t const *terms, size_t terms_size, clingo_literal_t const *condition, size_t condition_size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->theory_element(element_id, IdSpan(terms, terms_size), LiteralSpan(condition, condition_size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_theory_atom(clingo_id_t atom_id_or_zero, clingo_id_t term_id, clingo_id_t const *elements, size_t size, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->theory_atom(atom_id_or_zero, term_id, IdSpan(elements, size)); }
    GRINGO_CLINGO_CATCH;
}
bool g_theory_atom_with_guard(clingo_id_t atom_id_or_zero, clingo_id_t term_id, clingo_id_t const *elements, size_t size, clingo_id_t operator_id, clingo_id_t right_hand_side_id, GroundProgramObserver *self) {
    GRINGO_CLINGO_TRY { self->theory_atom_with_guard(atom_id_or_zero, term_id, IdSpan(elements, size), operator_id, right_hand_side_id); }
    GRINGO_CLINGO_CATCH;
}

static clingo_ground_program_observer_t g_observer = {
    reinterpret_cast<decltype(clingo_ground_program_observer_t::init_program)>(g_init_program),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::begin_step)>(g_begin_step),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::end_step)>(g_end_step),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::rule)>(g_rule),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::weight_rule)>(g_weight_rule),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::minimize)>(g_minimize),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::project)>(g_project),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::output_atom)>(g_output_atom),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::output_term)>(g_output_term),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::output_csp)>(g_output_csp),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::external)>(g_external),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::assume)>(g_assume),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::heuristic)>(g_heuristic),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::acyc_edge)>(g_acyc_edge),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::theory_term_number)>(g_theory_term_number),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::theory_term_string)>(g_theory_term_string),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::theory_term_compound)>(g_theory_term_compound),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::theory_element)>(g_theory_element),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::theory_atom)>(g_theory_atom),
    reinterpret_cast<decltype(clingo_ground_program_observer_t::theory_atom_with_guard)>(g_theory_atom_with_guard)
};

} // namespace

void Control::register_observer(GroundProgramObserver &observer, bool replace) {
    handleCError(clingo_control_register_observer(*impl_, g_observer, replace, &observer));
}

void Control::cleanup() {
    handleCError(clingo_control_cleanup(*impl_));
}

bool Control::has_const(char const *name) const {
    bool ret;
    handleCError(clingo_control_has_const(*impl_, name, &ret));
    return ret;
}

Symbol Control::get_const(char const *name) const {
    clingo_symbol_t ret;
    handleCError(clingo_control_get_const(*impl_, name, &ret));
    return Symbol(ret);
}

void Control::interrupt() noexcept {
    clingo_control_interrupt(*impl_);
}

void *Control::claspFacade() {
    void *ret;
    handleCError(clingo_control_clasp_facade(impl_->ctl, &ret));
    return ret;
}

void Control::load(char const *file) {
    handleCError(clingo_control_load(*impl_, file));
}

SolveAsync Control::solve_async(ModelCallback mh, FinishCallback fh, SymbolicLiteralSpan assumptions) {
    clingo_solve_async_t *ret;
    impl_->mh = std::move(mh);
    impl_->fh = std::move(fh);
    handleCError(clingo_control_solve_async(*impl_, [](clingo_model_t *m, void *data, bool *ret) -> bool {
        GRINGO_CLINGO_TRY {
            auto &mh = *static_cast<ModelCallback*>(data);
            *ret = !mh || mh(Model{m});
        }
        GRINGO_CLINGO_CATCH;
    }, &impl_->mh, [](clingo_solve_result_bitset_t res, void *data) -> bool {
        GRINGO_CLINGO_TRY {
            auto &fh = *static_cast<FinishCallback*>(data);
            if (fh) { fh(SolveResult{res}); }
        }
        GRINGO_CLINGO_CATCH;
    }, &impl_->fh, reinterpret_cast<clingo_symbolic_literal_t const *>(assumptions.begin()), assumptions.size(), &ret));
    return SolveAsync{ret};
}

void Control::use_enumeration_assumption(bool value) {
    handleCError(clingo_control_use_enumeration_assumption(*impl_, value));
}

Backend Control::backend() {
    clingo_backend_t *ret;
    handleCError(clingo_control_backend(*impl_, &ret));
    return Backend{ret};
}

Configuration Control::configuration() {
    clingo_configuration_t *conf;
    handleCError(clingo_control_configuration(*impl_, &conf));
    unsigned key;
    handleCError(clingo_configuration_root(conf, &key));
    return Configuration{conf, key};
}

Statistics Control::statistics() const {
    clingo_statistics_t *stats;
    handleCError(clingo_control_statistics(const_cast<clingo_control_t*>(impl_->ctl), &stats));
    uint64_t key;
    handleCError(clingo_statistics_root(stats, &key));
    return Statistics{stats, key};
}

ProgramBuilder Control::builder() {
    clingo_program_builder_t *ret;
    handleCError(clingo_control_program_builder(impl_->ctl, &ret));
    return ProgramBuilder{ret};
}

// {{{1 global functions

Symbol parse_term(char const *str, Logger logger, unsigned message_limit) {
    clingo_symbol_t ret;
    handleCError(clingo_parse_term(str, [](clingo_warning_t code, char const *msg, void *data) {
        try { (*static_cast<Logger*>(data))(static_cast<WarningCode>(code), msg); }
        catch (...) { }
    }, &logger, message_limit, &ret));
    return Symbol(ret);
}

char const *add_string(char const *str) {
    char const *ret;
    handleCError(clingo_add_string(str, &ret));
    return ret;
}

namespace AST {

namespace {

template <class T>
struct PrintWrapper {
    T const &vec;
    char const *pre;
    char const *sep;
    char const *post;
    bool empty;
    friend std::ostream &operator<<(std::ostream &out, PrintWrapper x) {
        using namespace std;
        auto it = std::begin(x.vec), ie = std::end(x.vec);
        if (it != ie) {
            out << x.pre;
            out << *it;
            for (++it; it != ie; ++it) {
                out << x.sep << *it;
            }
            out << x.post;
        }
        else if (x.empty) {
            out << x.pre;
            out << x.post;
        }
        return out;
    }
};


template <class T>
PrintWrapper<T> print(T const &vec, char const *pre, char const *sep, char const *post, bool empty) {
    return {vec, pre, sep, post, empty};
}

PrintWrapper<std::vector<BodyLiteral>> print_body(std::vector<BodyLiteral> const &vec, char const *pre = " : ") {
    return print(vec, vec.empty() ? "" : pre, "; ", ".", true);
}

// {{{2 C -> C++

#define ARR(in, out) \
std::vector<out> conv ## out ## Vec(in const *arr, size_t size) { \
    std::vector<out> ret; \
    for (auto it = arr, ie = arr + size; it != ie; ++it) { \
        ret.emplace_back(conv ## out(*it)); \
    } \
    return ret; \
}

// {{{3 terms

Id convId(clingo_ast_id_t const &id) {
    return {Location(id.location), id.id};
}
ARR(clingo_ast_id_t, Id)

Term convTerm(clingo_ast_term_t const &term);
ARR(clingo_ast_term_t, Term)

Term convTerm(clingo_ast_term_t const &term) {
    switch (static_cast<enum clingo_ast_term_type>(term.type)) {
        case clingo_ast_term_type_symbol: {
            return {Location{term.location}, Symbol{term.symbol}};
        }
        case clingo_ast_term_type_variable: {
            return {Location{term.location}, Variable{term.variable}};
        }
        case clingo_ast_term_type_unary_operation: {
            auto &op = *term.unary_operation;
            return {Location{term.location}, UnaryOperation{static_cast<UnaryOperator>(op.unary_operator), convTerm(op.argument)}};
        }
        case clingo_ast_term_type_binary_operation: {
            auto &op = *term.binary_operation;
            return {Location{term.location}, BinaryOperation{static_cast<BinaryOperator>(op.binary_operator), convTerm(op.left), convTerm(op.right)}};
        }
        case clingo_ast_term_type_interval: {
            auto &x = *term.interval;
            return {Location{term.location}, Interval{convTerm(x.left), convTerm(x.right)}};
        }
        case clingo_ast_term_type_function: {
            auto &x = *term.function;
            return {Location{term.location}, Function{x.name, convTermVec(x.arguments, x.size), false}};
        }
        case clingo_ast_term_type_external_function: {
            auto &x = *term.external_function;
            return {Location{term.location}, Function{x.name, convTermVec(x.arguments, x.size), true}};
        }
        case clingo_ast_term_type_pool: {
            auto &x = *term.pool;
            return {Location{term.location}, Pool{convTermVec(x.arguments, x.size)}};
        }
    }
    throw std::logic_error("cannot happen");
}

Optional<Term> convTerm(clingo_ast_term_t const *term) {
    return term ? Optional<Term>{convTerm(*term)} : Optional<Term>{};
}

// csp

CSPProduct convCSPProduct(clingo_ast_csp_product_term const &term) {
    return {Location{term.location}, convTerm(term.coefficient), convTerm(term.variable)};
}
ARR(clingo_ast_csp_product_term, CSPProduct)

CSPSum convCSPAdd(clingo_ast_csp_sum_term_t const &term) {
    return {Location{term.location}, convCSPProductVec(term.terms, term.size)};
}

// theory

TheoryTerm convTheoryTerm(clingo_ast_theory_term_t const &term);
ARR(clingo_ast_theory_term_t, TheoryTerm)

TheoryUnparsedTermElement convTheoryUnparsedTermElement(clingo_ast_theory_unparsed_term_element_t const &term) {
    return {std::vector<char const *>{term.operators, term.operators + term.size}, convTheoryTerm(term.term)};
}
ARR(clingo_ast_theory_unparsed_term_element_t, TheoryUnparsedTermElement)

TheoryTerm convTheoryTerm(clingo_ast_theory_term_t const &term) {
    switch (static_cast<enum clingo_ast_theory_term_type>(term.type)) {
        case clingo_ast_theory_term_type_symbol: {
            return {Location{term.location}, Symbol{term.symbol}};
        }
        case clingo_ast_theory_term_type_variable: {
            return {Location{term.location}, Variable{term.variable}};
        }
        case clingo_ast_theory_term_type_list: {
            auto &x = *term.list;
            return {Location{term.location}, TheoryTermSequence{TheoryTermSequenceType::List, convTheoryTermVec(x.terms, x.size)}};
        }
        case clingo_ast_theory_term_type_set: {
            auto &x = *term.list;
            return {Location{term.location}, TheoryTermSequence{TheoryTermSequenceType::Set, convTheoryTermVec(x.terms, x.size)}};
        }
        case clingo_ast_theory_term_type_tuple: {
            auto &x = *term.list;
            return {Location{term.location}, TheoryTermSequence{TheoryTermSequenceType::Tuple, convTheoryTermVec(x.terms, x.size)}};
        }
        case clingo_ast_theory_term_type_function: {
            auto &x = *term.function;
            return {Location{term.location}, TheoryFunction{x.name, convTheoryTermVec(x.arguments, x.size)}};
        }
        case clingo_ast_theory_term_type_unparsed_term: {
            auto &x = *term.unparsed_term;
            return {Location{term.location}, TheoryUnparsedTerm{convTheoryUnparsedTermElementVec(x.elements, x.size)}};
        }
    }
    throw std::logic_error("cannot happen");
}

// {{{3 literal

CSPGuard convCSPGuard(clingo_ast_csp_guard_t const &guard) {
    return {static_cast<ComparisonOperator>(guard.comparison), convCSPAdd(guard.term)};
}
ARR(clingo_ast_csp_guard_t, CSPGuard)

Literal convLiteral(clingo_ast_literal_t const &lit) {
    switch (static_cast<enum clingo_ast_literal_type>(lit.type)) {
        case clingo_ast_literal_type_boolean: {
            return {Location(lit.location), static_cast<Sign>(lit.sign), Boolean{lit.boolean}};
        }
        case clingo_ast_literal_type_symbolic: {
            return {Location(lit.location), static_cast<Sign>(lit.sign), convTerm(*lit.symbol)};
        }
        case clingo_ast_literal_type_comparison: {
            auto &c = *lit.comparison;
            return {Location(lit.location), static_cast<Sign>(lit.sign), Comparison{static_cast<ComparisonOperator>(c.comparison), convTerm(c.left), convTerm(c.right)}};
        }
        case clingo_ast_literal_type_csp: {
            auto &c = *lit.csp_literal;
            return {Location(lit.location), static_cast<Sign>(lit.sign), CSPLiteral{convCSPAdd(c.term), convCSPGuardVec(c.guards, c.size)}};
        }
    }
    throw std::logic_error("cannot happen");
}
ARR(clingo_ast_literal_t, Literal)

// {{{3 aggregates

Optional<AggregateGuard> convAggregateGuard(clingo_ast_aggregate_guard_t const *guard) {
    return guard
        ? Optional<AggregateGuard>{AggregateGuard{static_cast<ComparisonOperator>(guard->comparison), convTerm(guard->term)}}
        : Optional<AggregateGuard>{};
}

ConditionalLiteral convConditionalLiteral(clingo_ast_conditional_literal_t const &lit) {
    return {convLiteral(lit.literal), convLiteralVec(lit.condition, lit.size)};
}
ARR(clingo_ast_conditional_literal_t, ConditionalLiteral)

Aggregate convAggregate(clingo_ast_aggregate_t const &aggr) {
    return {convConditionalLiteralVec(aggr.elements, aggr.size), convAggregateGuard(aggr.left_guard), convAggregateGuard(aggr.right_guard)};
}

// theory atom

Optional<TheoryGuard> convTheoryGuard(clingo_ast_theory_guard_t const *guard) {
    return guard
        ? Optional<TheoryGuard>{TheoryGuard{guard->operator_name, convTheoryTerm(guard->term)}}
        : Optional<TheoryGuard>{};
}

TheoryAtomElement convTheoryAtomElement(clingo_ast_theory_atom_element_t const &elem) {
    return {convTheoryTermVec(elem.tuple, elem.tuple_size), convLiteralVec(elem.condition, elem.condition_size)};
}
ARR(clingo_ast_theory_atom_element_t, TheoryAtomElement)

TheoryAtom convTheoryAtom(clingo_ast_theory_atom_t const &atom) {
    return {convTerm(atom.term), convTheoryAtomElementVec(atom.elements, atom.size), convTheoryGuard(atom.guard)};
}

// disjoint

DisjointElement convDisjointElement(clingo_ast_disjoint_element_t const &elem) {
    return {Location{elem.location}, convTermVec(elem.tuple, elem.tuple_size), convCSPAdd(elem.term), convLiteralVec(elem.condition, elem.condition_size)};
}
ARR(clingo_ast_disjoint_element_t, DisjointElement)

// head aggregates

HeadAggregateElement convHeadAggregateElement(clingo_ast_head_aggregate_element_t const &elem) {
    return {convTermVec(elem.tuple, elem.tuple_size), convConditionalLiteral(elem.conditional_literal)};
}
ARR(clingo_ast_head_aggregate_element_t, HeadAggregateElement)

// body aggregates

BodyAggregateElement convBodyAggregateElement(clingo_ast_body_aggregate_element_t const &elem) {
    return {convTermVec(elem.tuple, elem.tuple_size), convLiteralVec(elem.condition, elem.condition_size)};
}
ARR(clingo_ast_body_aggregate_element_t, BodyAggregateElement)

// {{{3 head literal

HeadLiteral convHeadLiteral(clingo_ast_head_literal_t const &head) {
    switch (static_cast<enum clingo_ast_head_literal_type>(head.type)) {
        case clingo_ast_head_literal_type_literal: {
            return {Location{head.location}, convLiteral(*head.literal)};
        }
        case clingo_ast_head_literal_type_disjunction: {
            auto &d = *head.disjunction;
            return {Location{head.location}, Disjunction{convConditionalLiteralVec(d.elements, d.size)}};
        }
        case clingo_ast_head_literal_type_aggregate: {
            return {Location{head.location}, convAggregate(*head.aggregate)};
        }
        case clingo_ast_head_literal_type_head_aggregate: {
            auto &a = *head.head_aggregate;
            return {Location{head.location}, HeadAggregate{static_cast<AggregateFunction>(a.function), convHeadAggregateElementVec(a.elements, a.size), convAggregateGuard(a.left_guard), convAggregateGuard(a.right_guard)}};
        }
        case clingo_ast_head_literal_type_theory_atom: {
            return {Location{head.location}, convTheoryAtom(*head.theory_atom)};
        }
    }
    throw std::logic_error("cannot happen");
}

// {{{3 body literal

BodyLiteral convBodyLiteral(clingo_ast_body_literal_t const &body) {
    switch (static_cast<enum clingo_ast_body_literal_type>(body.type)) {
        case clingo_ast_body_literal_type_literal: {
            return {Location{body.location}, static_cast<Sign>(body.sign), convLiteral(*body.literal)};
        }
        case clingo_ast_body_literal_type_conditional: {
            return {Location{body.location}, static_cast<Sign>(body.sign), convConditionalLiteral(*body.conditional)};
        }
        case clingo_ast_body_literal_type_aggregate: {
            return {Location{body.location}, static_cast<Sign>(body.sign), convAggregate(*body.aggregate)};
        }
        case clingo_ast_body_literal_type_body_aggregate: {
            auto &a = *body.body_aggregate;
            return {Location{body.location}, static_cast<Sign>(body.sign), BodyAggregate{static_cast<AggregateFunction>(a.function), convBodyAggregateElementVec(a.elements, a.size), convAggregateGuard(a.left_guard), convAggregateGuard(a.right_guard)}};
        }
        case clingo_ast_body_literal_type_theory_atom: {
            return {Location{body.location}, static_cast<Sign>(body.sign), convTheoryAtom(*body.theory_atom)};
        }
        case clingo_ast_body_literal_type_disjoint: {
            auto &d = *body.disjoint;
            return {Location{body.location}, static_cast<Sign>(body.sign), Disjoint{convDisjointElementVec(d.elements, d.size)}};
        }
    }
    throw std::logic_error("cannot happen");
}
ARR(clingo_ast_body_literal_t, BodyLiteral)

// {{{3 statement

TheoryOperatorDefinition convTheoryOperatorDefinition(clingo_ast_theory_operator_definition_t const &def) {
    return {Location{def.location}, def.name, def.priority, static_cast<TheoryOperatorType>(def.type)};
}
ARR(clingo_ast_theory_operator_definition_t, TheoryOperatorDefinition)

Optional<TheoryGuardDefinition> convTheoryGuardDefinition(clingo_ast_theory_guard_definition_t const *def) {
    return def
        ? Optional<TheoryGuardDefinition>{def->term, std::vector<char const *>{def->operators, def->operators + def->size}}
        : Optional<TheoryGuardDefinition>{};
}

TheoryTermDefinition convTheoryTermDefinition(clingo_ast_theory_term_definition_t const &def) {
    return {Location{def.location}, def.name, convTheoryOperatorDefinitionVec(def.operators, def.size)};
    std::vector<TheoryOperatorDefinition> operators;
}
ARR(clingo_ast_theory_term_definition_t, TheoryTermDefinition)

TheoryAtomDefinition convTheoryAtomDefinition(clingo_ast_theory_atom_definition_t const &def) {
    return {Location{def.location}, static_cast<TheoryAtomDefinitionType>(def.type), def.name, def.arity, def.elements, convTheoryGuardDefinition(def.guard)};
}
ARR(clingo_ast_theory_atom_definition_t, TheoryAtomDefinition)

void convStatement(clingo_ast_statement_t const *stm, StatementCallback &cb) {
    switch (static_cast<enum clingo_ast_statement_type>(stm->type)) {
        case clingo_ast_statement_type_rule: {
            cb({Location(stm->location), Rule{convHeadLiteral(stm->rule->head), convBodyLiteralVec(stm->rule->body, stm->rule->size)}});
            break;
        }
        case clingo_ast_statement_type_const: {
            cb({Location(stm->location), Definition{stm->definition->name, convTerm(stm->definition->value), stm->definition->is_default}});
            break;
        }
        case clingo_ast_statement_type_show_signature: {
            cb({Location(stm->location), ShowSignature{Signature(stm->show_signature->signature), stm->show_signature->csp}});
            break;
        }
        case clingo_ast_statement_type_show_term: {
            cb({Location(stm->location), ShowTerm{convTerm(stm->show_term->term), convBodyLiteralVec(stm->show_term->body, stm->show_term->size), stm->show_term->csp}});
            break;
        }
        case clingo_ast_statement_type_minimize: {
            auto &min = *stm->minimize;
            cb({Location(stm->location), Minimize{convTerm(min.weight), convTerm(min.priority), convTermVec(min.tuple, min.tuple_size), convBodyLiteralVec(min.body, min.body_size)}});
            break;
        }
        case clingo_ast_statement_type_script: {
            cb({Location(stm->location), Script{static_cast<ScriptType>(stm->script->type), stm->script->code}});
            break;
        }
        case clingo_ast_statement_type_program: {
            cb({Location(stm->location), Program{stm->program->name, convIdVec(stm->program->parameters, stm->program->size)}});
            break;
        }
        case clingo_ast_statement_type_external: {
            cb({Location(stm->location), External{convTerm(stm->external->atom), convBodyLiteralVec(stm->external->body, stm->external->size)}});
            break;
        }
        case clingo_ast_statement_type_edge: {
            cb({Location(stm->location), Edge{convTerm(stm->edge->u), convTerm(stm->edge->v), convBodyLiteralVec(stm->edge->body, stm->edge->size)}});
            break;
        }
        case clingo_ast_statement_type_heuristic: {
            auto &heu = *stm->heuristic;
            cb({Location(stm->location), Heuristic{convTerm(heu.atom), convBodyLiteralVec(heu.body, heu.size), convTerm(heu.bias), convTerm(heu.priority), convTerm(heu.modifier)}});
            break;
        }
        case clingo_ast_statement_type_project_atom: {
            cb({Location(stm->location), ProjectAtom{convTerm(stm->project_atom->atom), convBodyLiteralVec(stm->project_atom->body, stm->project_atom->size)}});
            break;
        }
        case clingo_ast_statement_type_project_atom_signature: {
            cb({Location(stm->location), ProjectSignature{Signature(stm->project_signature)}});
            break;
        }
        case clingo_ast_statement_type_theory_definition: {
            auto &def = *stm->theory_definition;
            cb({Location(stm->location), TheoryDefinition{def.name, convTheoryTermDefinitionVec(def.terms, def.terms_size), convTheoryAtomDefinitionVec(def.atoms, def.atoms_size)}});
            break;
        }
    }
}

#undef ARR

// }}}3

// }}}2

} // namespace

// {{{2 printing

// {{{3 statement

std::ostream &operator<<(std::ostream &out, TheoryDefinition const &x) {
    out << "#theory " << x.name << " {\n";
    bool comma = false;
    for (auto &y : x.terms) {
        if (comma) { out << ";\n"; }
        else       { comma = true; }
        out << "  " << y.name << " {\n" << print(y.operators, "    ", ";\n", "\n", true) << "  }";
    }
    for (auto &y : x.atoms) {
        if (comma) { out << ";\n"; }
        else       { comma = true; }
        out << "  " << y;
    }
    if (comma) { out << "\n"; }
    out << "}.";
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryAtomDefinition const &x) {
    out << "&" << x.name << "/" << x.arity << " : " << x.elements;
    if (x.guard) { out << ", " << *x.guard.get(); }
    out << ", " << x.type;
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryGuardDefinition const &x) {
    out << "{ " << print(x.operators, "", ", ", "", false) << " }, " << x.term;
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryTermDefinition const &x) {
    out << x.name << " {\n" << print(x.operators, "  ", ";\n", "\n", true) << "}";
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryOperatorDefinition const &x) {
    out << x.name << " : " << x.priority << ", " << x.type;
    return out;
}

std::ostream &operator<<(std::ostream &out, BodyLiteral const &x) {
    out << x.sign << x.data;
    return out;
}

std::ostream &operator<<(std::ostream &out, HeadLiteral const &x) {
    out << x.data;
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryAtom const &x) {
    out << "&" << x.term << " { " << print(x.elements, "", "; ", "", false) << " }";
    if (x.guard) { out << " " << *x.guard.get(); }
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryGuard const &x) {
    out << x.operator_name << " " << x.term;
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryAtomElement const &x) {
    out << print(x.tuple, "", ",", "", false) << " : " << print(x.condition, "", ",", "", false);
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryUnparsedTermElement const &x) {
    out << print(x.operators, " ", " ", " ", false) << x.term;
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryFunction const &x) {
    out << x.name << print(x.arguments, "(", ",", ")", !x.arguments.empty());
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryTermSequence const &x) {
    bool tc = x.terms.size() == 1 && x.type == TheoryTermSequenceType::Tuple;
    out << print(x.terms, left_hand_side(x.type), ",", "", true);
    if (tc) { out << ",)"; }
    else    { out << right_hand_side(x.type); }
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryTerm const &x) {
    out << x.data;
    return out;
}

std::ostream &operator<<(std::ostream &out, TheoryUnparsedTerm const &x) {
    if (x.elements.size() > 1) { out << "("; }
    out << print(x.elements, "", "", "", false);
    if (x.elements.size() > 1) { out << ")"; }
    return out;
}

std::ostream &operator<<(std::ostream &out, Disjoint const &x) {
    out << "#disjoint { " << print(x.elements, "", "; ", "", false) << " }";
    return out;
}

std::ostream &operator<<(std::ostream &out, DisjointElement const &x) {
    out << print(x.tuple, "", ",", "", false) << " : " << x.term << " : " << print(x.condition, "", ",", "", false);
    return out;
}

std::ostream &operator<<(std::ostream &out, Disjunction const &x) {
    out << print(x.elements, "", "; ", "", false);
    return out;
}

std::ostream &operator<<(std::ostream &out, HeadAggregate const &x) {
    if (x.left_guard) { out << x.left_guard->term << " " << x.left_guard->comparison << " "; }
    out << x.function << " { " << print(x.elements, "", "; ", "", false) << " }";
    if (x.right_guard) { out << " " << x.right_guard->comparison << " " << x.right_guard->term; }
    return out;
}

std::ostream &operator<<(std::ostream &out, HeadAggregateElement const &x) {
    out << print(x.tuple, "", ",", "", false) << " : " << x.condition;
    return out;
}

std::ostream &operator<<(std::ostream &out, BodyAggregate const &x) {
    if (x.left_guard) { out << x.left_guard->term << " " << x.left_guard->comparison << " "; }
    out << x.function << " { " << print(x.elements, "", "; ", "", false) << " }";
    if (x.right_guard) { out << " " << x.right_guard->comparison << " " << x.right_guard->term; }
    return out;
}

std::ostream &operator<<(std::ostream &out, BodyAggregateElement const &x) {
    out << print(x.tuple, "", ",", "", false) << " : " << print(x.condition, "", ", ", "", false);
    return out;
}

std::ostream &operator<<(std::ostream &out, Aggregate const &x) {
    if (x.left_guard) { out << x.left_guard->term << " " << x.left_guard->comparison << " "; }
    out << "{ " << print(x.elements, "", "; ", "", false) << " }";
    if (x.right_guard) { out << " " << x.right_guard->comparison << " " << x.right_guard->term; }
    return out;
}

std::ostream &operator<<(std::ostream &out, ConditionalLiteral const &x) {
    out << x.literal << print(x.condition, " : ", ", ", "", true);
    return out;
}

std::ostream &operator<<(std::ostream &out, Literal const &x) {
    out << x.sign << x.data;
    return out;
}

std::ostream &operator<<(std::ostream &out, Boolean const &x) {
    out << (x.value ? "#true" : "#false");
    return out;
}

std::ostream &operator<<(std::ostream &out, Comparison const &x) {
    out << x.left << x.comparison << x.right;
    return out;
}

std::ostream &operator<<(std::ostream &out, Id const &x) {
    out << x.id;
    return out;
}

std::ostream &operator<<(std::ostream &out, CSPLiteral const &x) {
    out << x.term;
    for (auto &y : x.guards) { out << y; }
    return out;
}

std::ostream &operator<<(std::ostream &out, CSPGuard const &x) {
    out << "$" << x.comparison << x.term;
    return out;
}

std::ostream &operator<<(std::ostream &out, CSPSum const &x) {
    if (x.terms.empty()) { out << "0"; }
    else                 { out << print(x.terms, "", "$+", "", false); }
    return out;
}

std::ostream &operator<<(std::ostream &out, CSPProduct const &x) {
    if (x.variable) { out << x.coefficient << "$*$" << *x.variable.get(); }
    else            { out << x.coefficient; }
    return out;
}

std::ostream &operator<<(std::ostream &out, Pool const &x) {
    // NOTE: there is no representation for an empty pool
    if (x.arguments.empty()) { out << "(1/0)"; }
    else                     { out << print(x.arguments, "(", ";", ")", true); }
    return out;
}

std::ostream &operator<<(std::ostream &out, Function const &x) {
    bool tc = x.name[0] == '\0' && x.arguments.size() == 1;
    bool ey = x.name[0] == '\0' || !x.arguments.empty();
    out << (x.external ? "@" : "") << x.name << print(x.arguments, "(", ",", tc ? ",)" : ")", ey);
    return out;
}

std::ostream &operator<<(std::ostream &out, Interval const &x) {
    out << "(" << x.left << ".." << x.right << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, BinaryOperation const &x) {
    out << "(" << x.left << x.binary_operator << x.right << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, UnaryOperation const &x) {
    out << left_hand_side(x.unary_operator) << x.argument << right_hand_side(x.unary_operator);
    return out;
}

std::ostream &operator<<(std::ostream &out, Variable const &x) {
    out << x.name;
    return out;
}

std::ostream &operator<<(std::ostream &out, Term const &x) {
    out << x.data;
    return out;
}

std::ostream &operator<<(std::ostream &out, Rule const &x) {
    out << x.head << print_body(x.body, " :- ");
    return out;
}

std::ostream &operator<<(std::ostream &out, Definition const &x) {
    out << "#const " << x.name << " = " << x.value << ".";
    if (x.is_default) { out << " [default]"; }
    return out;
}

std::ostream &operator<<(std::ostream &out, ShowSignature const &x) {
    out << "#show " << (x.csp ? "$" : "") << x.signature << ".";
    return out;
}

std::ostream &operator<<(std::ostream &out, ShowTerm const &x) {
    out << "#show " << (x.csp ? "$" : "") << x.term << print_body(x.body);
    return out;
}

std::ostream &operator<<(std::ostream &out, Minimize const &x) {
    out << print_body(x.body, ":~ ") << " [" << x.weight << "@" << x.priority << print(x.tuple, ",", ",", "", false) << "]";
    return out;
}

std::ostream &operator<<(std::ostream &out, Script const &x) {
    std::string s = x.code;
    if (!s.empty() && s.back() == '\n') {
        s.back() = '.';
    }
    out << s;
    return out;
}

std::ostream &operator<<(std::ostream &out, Program const &x) {
    out << "#program " << x.name << print(x.parameters, "(", ",", ")", false) << ".";
    return out;
}

std::ostream &operator<<(std::ostream &out, External const &x) {
    out << "#external " << x.atom << print_body(x.body);
    return out;
}

std::ostream &operator<<(std::ostream &out, Edge const &x) {
    out << "#edge (" << x.u << "," << x.v << ")" << print_body(x.body);
    return out;
}

std::ostream &operator<<(std::ostream &out, Heuristic const &x) {
    out << "#heuristic " << x.atom << print_body(x.body) << " [" << x.bias<< "@" << x.priority << "," << x.modifier << "]";
    return out;
}

std::ostream &operator<<(std::ostream &out, ProjectAtom const &x) {
    out << "#project " << x.atom << print_body(x.body);
    return out;
}

std::ostream &operator<<(std::ostream &out, ProjectSignature const &x) {
    out << "#project " << x.signature << ".";
    return out;
}

std::ostream &operator<<(std::ostream &out, Statement const &x) {
    out << x.data;
    return out;
}
// }}}3

// }}}2

} // namespace AST

void parse_program(char const *program, StatementCallback cb, Logger logger, unsigned message_limit) {
    using Data = std::pair<StatementCallback &, std::exception_ptr>;
    Data data(cb, nullptr);
    handleCError(clingo_parse_program(program, [](clingo_ast_statement_t const *stm, void *data) -> bool {
        auto &d = *static_cast<Data*>(data);
        CLINGO_CALLBACK_TRY { AST::convStatement(stm, d.first); }
        CLINGO_CALLBACK_CATCH(d.second);
    }, &data, [](clingo_warning_t code, char const *msg, void *data) {
        try { (*static_cast<Logger*>(data))(static_cast<WarningCode>(code), msg); }
        catch (...) { }
    }, &logger, message_limit));
}

// }}}1

} // namespace Clingo