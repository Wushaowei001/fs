
#pragma once

#include <memory>
#include <vector>
#include <set>
#include <constraints/scoped_constraint.hxx>

namespace fs0 {


/**
 * An alldifferent custom propagator. Currently supports only Puget's bound consistency algorithm 2 from
 * 
 * Puget, J. F. (1998, July). A fast algorithm for the bound consistency of alldiff constraints. In AAAI/IAAI (pp. 359-366).
 * 
 * with a complexity of O(n^2), where n is the number of variables.
 */
class ScopedAlldiffConstraint : public ScopedConstraint
{
protected:
	//! The arity of the constraint
	unsigned _arity;
	
	//! Vectors to store min and max domain values.
	std::vector<int> min, max;
	
	//! The variables sorted by increasing max domain value
	std::vector<VariableIdx> sorted_vars;
	
	//! The variables sorted by increasing max domain value
	std::vector<int> u;

public:
	ScopedAlldiffConstraint(const VariableIdxVector& scope);
	ScopedAlldiffConstraint(const VariableIdxVector& scope, const std::vector<int>& parameters);
	
	virtual ~ScopedAlldiffConstraint() {}
	
	bool isSatisfied(const ObjectIdxVector& values) const;

	//! Filters from the set of currently loaded projections
	// Computing bound consistent domains is done in two passes. The algorithm that computes new
	// min is applied twice: first to the original problem, resulting into new min bounds, second to the problem
	// where variables are replaced by their inverse, deducing max bounds.
	Output filter();
	
protected:
	//! Invert a domain, e.g. from D = {3, 4, 7} to D = {-7, -4, -3}
	Domain invertDomain(const Domain& domain) const;
	
	//! Invert all the variable domains.
	void invertDomains(const DomainVector& domains);
	
	//! Sort the variables in increasing order of the max value of their domain, leaving them in the `sorted_vars` attribute.
	void sortVariables(const DomainVector& domains);
	
	void updateBounds(const DomainVector& domains);
	
	//! [a,b] is a Hall interval
	Output incrMin(const DomainVector& domains, int a, int b, unsigned i);

	Output insert(const DomainVector& domains, unsigned i);
	
	Output bounds_consistency(const DomainVector& domains);
	
};


} // namespaces
