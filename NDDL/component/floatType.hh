#ifndef FLOAT_TYPE_HH
#define FLOAT_TYPE_HH

#include "IntervalDomain.hh"
#include "TypeFactory.hh"

namespace Prototype {

  /**
   * @class floatDomain
   * @brief same as IntervalDomain, except with the NDDL-specific "float" type name.
   */
  class floatDomain : public IntervalDomain {
  public:
    floatDomain();

    /**
     * @brief Get the name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    virtual const LabelStr& getTypeName() const;
  };

  class floatTypeFactory : public ConcreteTypeFactory {
  public:
    floatTypeFactory();

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constaintEngine, 
                                                 const LabelStr& variableName) const;

    /**
     * @brief Create a domain
     */
    virtual AbstractDomain * createDomain() const;

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(std::string value) const;

  };

} // namespace Prototype

#endif // FLOAT_TYPE_HH
