
#pragma once

#include <iostream>
#include <cstdlib>
#include <memory>
#include <set>
#include <vector>
#include <boost/functional/hash.hpp>

#include <fs0_types.hxx>
#include <atoms.hxx>

namespace fs0 {

class Problem;

class State {
protected:
	//! A vector associating symbol IDs to their current extensional value in the state.
	std::vector<ObjectIdx> _values;

	std::size_t _hash;

public:
	friend class RelaxedState;
	
	typedef std::shared_ptr<State> ptr;
	typedef std::shared_ptr<const State> cptr;

	virtual ~State() {}
	
	//! Construct a state specifying all the predicates and functions - this should indeed be the only way,
	//! although ATM we are forced to leave the no-arguments constructor in order to be able
	//! to set a Problem's initial state after construction
	//! Note that it is not necessarily the case that numAtoms == facts.size(); since the initial values of
	//! some (Boolean) state variables is often left unspecified and understood to be false.
	//! TODO - We might want to perform type checking here against the predicate and function signatures.
	//! TODO - We might also want to ensure here that all symbol extensions have been defined. This won't be expensive, 
	//! as it will be done only when we create the initial state.
	State(unsigned numAtoms, const Atom::vctr& facts) :
		_values(numAtoms)
	{
		// Note that those facts not explicitly set in the initial state will be initialized to 0, i.e. "false", which is convenient to us.
		for (const auto& fact:facts) { // Insert all the elements of the vector
			set(fact);
		}
		updateHash();
	};
	
	
	//! Copy constructor
	State(const State& state) :
		_values(state._values), _hash(state._hash)
	{}
	
	//! A constructor that receives a number of atoms and constructs a state that is equal to the received
	//! state plus the new atoms. Note that we do not check that there are no contradictory atoms.
	State(const State& state, const Atom::vctr& atoms) :
		_values(state._values), _hash(state._hash) {
		accumulate(atoms);
	}
	
	//! Assignment operator
	// TODO - This is probably not exception-safe
	State& operator=(const State &rhs) {
		if (this == &rhs) return *this;
		_values = rhs._values;
		_hash = rhs._hash;
		return *this;
	}

	//! Move constructor
	State( State&& state ) {
		// MRJ: I love this sh*t
		_values = std::move(state._values);
		_hash = state._hash;
	}

	//! Assignment operator by move
	State&	operator=( State&& state ) {
		_values = std::move( state._values );
		_hash = state._hash;
		return *this;
	}

	
	bool operator==(const State &rhs) const {
		return _hash == rhs._hash && _values == rhs._values; // Check the hash first for performance.
	}
	
	bool operator!=(const State &rhs) const { return !(this->operator==(rhs));}
	
	void set(const Atom& atom) {
		_values.at(atom.getVariable()) = atom.getValue();
	}
	
	bool contains(const Atom& atom) const {
		return getValue(atom.getVariable()) == atom.getValue();
	}
	
	ObjectIdx getValue(const VariableIdx& variable) const {
		return _values.at(variable);
	}

	unsigned	numAtoms() const { return _values.size(); }
	
protected:
	//! "Applies" the given atoms into the current state.
	void accumulate(const Atom::vctr& atoms);
	
	void updateHash() { _hash = computeHash(); }
	
	std::size_t computeHash() const { return boost::hash_range(_values.begin(), _values.end()); };
	
public:
	//! Prints a representation of the state to the given stream.
	friend std::ostream& operator<<(std::ostream &os, const State&  state) { return state.print(os); }
	std::ostream& print(std::ostream& os) const;
	
	std::size_t hash() const { return _hash; }
	
	//! Required by Boost.Functional/Hash - Currently we don't need it, if we needed it, it might be better to return _hash directly,
	// as long as there is no circular dependency.
	// friend std::size_t hash_value(PredicativeState const& s) { return s.computeHash(); }
};

} // namespaces
