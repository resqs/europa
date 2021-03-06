#ifndef H_PlanDatabase
#define H_PlanDatabase
/**
 * @file   PlanDatabase.hh
 * @author Conor McGann
 * @date   Mon Dec 27 17:09:14 2004
 * @brief
 * @ingroup PlanDatabase
 */

#include "Debug.hh"
#include "Object.hh"
#include "PlanDatabaseDefs.hh"
#include "PSPlanDatabase.hh"
#include "PlanDatabaseListener.hh"
#include "Schema.hh"
#include "DbClient.hh"
#include "Engine.hh"

#include <set>
#include <map>
#include <list>
#include <vector>
#include <typeinfo>

namespace EUROPA {

	class ObjectVariableListener;

  /**
   * @brief The main mediator for interaction with entities of the plan and managing their relationships.
   */
  class PlanDatabase : public PSPlanDatabase {
  public:

    enum Event { TOKEN_ADDED = 0,
                 TOKEN_REMOVED,
                 TOKEN_CLOSED,
                 TOKEN_ACTIVATED,
                 TOKEN_DEACTIVATED,
                 TOKEN_MERGED,
                 TOKEN_SPLIT,
                 TOKEN_REJECTED,
                 TOKEN_REINSTATED,
                 OBJECT_ADDED,
                 OBJECT_REMOVED
    };

    enum State { OPEN = 0,
                 CLOSED,
                 PURGED
    };

    PlanDatabase(const ConstraintEngineId constraintEngine, const SchemaId schema);

    ~PlanDatabase();

    const PlanDatabaseId getId() const;

    const ConstraintEngineId getConstraintEngine() const;

    const SchemaId getSchema() const;

    const TemporalAdvisorId getTemporalAdvisor();

    void setTemporalAdvisor(const TemporalAdvisorId temporalAdvisor);

    /**
     * @brief Retrieve a client interface which provides an interception point for all transactions.
     */
    const DbClientId getClient() const;

    /**
     * @brief Retrieve an entity given its key value.
     * @return The base class reference for the entity. It can be casted to the expected type.
     * @see Id::convertable
     */
    EntityId getEntityByKey(int key) const;

    /**
     * @brief Returns the set of all objects in the database.
     *
     * @return All objects.
     */
    const ObjectSet& getObjects() const;

    /**
     * @brief Returns the set of all tokens in the database.
     * @return All Tokens
     */
    const TokenSet& getTokens() const;

    /**
     * @brief Returns the set of all active tokens by predicate.
     */
    const TokenSet& getActiveTokens(const std::string& predicate) const;

    /**
     * @brief Register an allocated global variable.
     * @param var The variable to be registered. Must not have a parent. Furthermore, the name of the variable must be unique in this scope.
     */
    void registerGlobalVariable(const ConstrainedVariableId var);

    /**
     * @brief Un-register an allocated global variable.
     */
    void unregisterGlobalVariable(const ConstrainedVariableId var);

    /**
     * @brief Return all global variables registered with the plan database as such
     * @return The set of all registered global variables.
     * @see registerGlobalVariable, DbClient::createVariable
     */
    const ConstrainedVariableSet& getGlobalVariables() const;

    /**
     * @brief Access a global variable by name.
     * @return The variable. It is an error if the variable is not present, or is corrupt in any way.
     */
    const ConstrainedVariableId getGlobalVariable(const std::string& varName) const;

    /**
     * @brief Test of a global exists for a given name
     * @return true of present, otherwise false.
     */
    bool isGlobalVariable(const std::string& varName) const;

    void registerGlobalToken(const TokenId t);
    void unregisterGlobalToken(const TokenId t);
    const TokenSet& getGlobalTokens() const;
    const TokenId getGlobalToken(const std::string& varName) const;
    bool isGlobalToken(const std::string& varName) const;

    /**
     * @brief Test if the given inactive token has any tokens with which it can merge.
     * @param inactiveToken The token to check for merge candidates.
     * @return true if there is 1 or more tokens with which to merge, otherwse false.
     * @see getCompatibleTokens, countCompatibleTokens
     */
    bool hasCompatibleTokens(const TokenId inactiveToken);

    /**
     * @brief Retrieves a collection of all Tokens that are compatible with the given token.
     * Database must be constraintConsistent.
     * @param inactiveToken The token to drive the search. It must be inActive().
     * @param results A (initially empty) collection to be populated with all compatible tokens.
     * All returned elements must be active().
     */
    void getCompatibleTokens(const TokenId inactiveToken,
			     std::vector<TokenId>& results);

    /**
     * @brief Retrieves a collection of all Tokens that are compatible with the given token.
     * Database must be constraintConsistent.
     * @param inactiveToken The token to drive the search. It must be inActive().
     * @param results A (initially empty) collection to be populated with all compatible tokens.
     * All returned elements must be active().
     * @param limit Sets a limit to the quantity returned. Use to reduce cost of getting choices.
     * @param useExactTest If true, use a much more expensive but more rigorous comparison of timepoints.
     */
    void getCompatibleTokens(const TokenId inactiveToken,
			     std::vector<TokenId>& results,
			     unsigned int limit,
			     bool useExactTest);
//     void getCompatibleTokens(const TokenId inactiveToken,
// 			     std::vector<TokenId>& results,
// 			     eint limit,
// 			     bool useExactTest);

    /**
     * @brief Returns a count of compatible tokens up to the given limit
     * @see getCompatibleTokens
     */
    unsigned long countCompatibleTokens(const TokenId inactiveToken,
#ifdef _MSC_VER
                                        unsigned int limit = UINT_MAX,  //std::numeric_limits<unsigned int>::max(),
#else
                                        unsigned int limit = std::numeric_limits<unsigned int>::max(),
#endif //_MSC_VER
                                        bool useExactTest = false);

    /**
     * @brief Returns the last count obtained for the prior call to query the set of compatible tokens. This can be useful
     * as a 'probable' indicator of the count of compatible tokens in the next call.
     * @see countCompatibleTokens, getCompatibleTokens
     */
    unsigned int lastCompatibleTokenCount(const TokenId inactiveToken) const;

    /**
     * @brief Retrieves the map relating token keys and the set of objects that induce an ordering requirement on the token.
     * @see notifyOrderingRequired, notifyOrderingNoLongerRequired
     */
    const std::map< eint, std::pair<TokenId, ObjectSet> >& getTokensToOrder();

    /**
     * @brief Retrieves the set of ordering choices for resolving a token which requires ordering
     * @param tokenToOrder The token to order. It's key must be in the set given by getTokensToOrder
     * @param results The collection of <objectId, <successor, predecessor> > tuples
     * @param limit An upper bound on the number of choices to return.
     * @see getTokensToOrder
     */
    void getOrderingChoices(const TokenId tokenToOrder,
			    std::vector< OrderingChoice >& results,
#ifdef _MSC_VER
			    unsigned int limit = UINT_MAX  //std::numeric_limits<unsigned int>::max(),
#else
			    unsigned int limit = std::numeric_limits<unsigned int>::max()
#endif //_MSC_VER
			    );
    /**
     * @brief Returns a count or all ordering choices for a token up to the given limit
     * @see Object::getOrderingChoices
     */
    unsigned long countOrderingChoices(const TokenId token,
#ifdef _MSC_VER
				      unsigned long limit = UINT_MAX  //std::numeric_limits<unsigned int>::max(),
#else
				      unsigned long limit = std::numeric_limits<unsigned long>::max()
#endif //_MSC_VER
				      );
    /**
     * @brief Returns the previous count of ordering choices
     * @param token The token for which we want ordering choices. Cannot be rejected.
     * @return The last count value
     */
    unsigned long lastOrderingChoiceCount(const TokenId token) const;

    /**
     * @brief True if there is at least one ordering choice
     */
    bool hasOrderingChoice(const TokenId token);

    /**
     * @brief Retrieves a collection of object instances of the given type. Database must be closed.
     * @param type The type of object sought.
     * @param results A (initially empty) collection to be populated with all instances of that type.
     */
    template<class ID>
    void getObjectsByType(const std::string& type, std::list<ID>& results);

    /**
     * @brief Are there any object instances of the specified type?
     */
    bool hasObjectInstances(const std::string& objectType) const;

    /**
     * @brief Lookup an object by name. It is an error if the object is not present.
     * @return The requested object
     */
    const ObjectId getObject(const std::string& name) const;

    /**
     * @brief Specifies that no more objects are to be added to the database.
     */
    void close();

    /**
     * @brief Specifies that no more objects of the given type are to be added to the database.
     *
     * Note that this will cause an error if Tokens have already been created for this type. This is
     * because we have the rule - once dynamic, always dynamic.
     */
    void close(const std::string& objectType);

    /**
     * @brief True if and only if close() has been called.
     */
    bool isClosed() const;

    /**
     * @brief True if close() has been called, or close(objectType) has been called
     */
    bool isClosed(const std::string& objectType) const;

    /**
     * @brief True if close() has not been called. This is because the schema cannot be assumed to be fixed, and we therefore
     * cannot otherwise conclude that all types have been closed.
     */
    bool isOpen() const;

    State getState() const;

    bool isPurged() const;

    /**
     * Delete all Objects and Tokens
     */
    void purge();

    /**
     * @brief Make ObjectVariable. Will populate initial values and
     * close the variable. Will also hook up the variable for synchronization
     * if the type for the variable is open.
     * @param objectType The type of objects to pull from
     * @param objectVar The variable to be populated and possibly synchronized. Must be open.
     * @param leaveOpen If true, the object var will remain open on completion.
     */
    void makeObjectVariableFromType(const std::string& objectType,
				    const ConstrainedVariableId objectVar,
				    bool leaveOpen = false);

    /**
     * @brief Archive all tokens whose end times precede the given tick in time. Basically nuking a portin of the
     * database. The database must be constraintConsistent
     * @param tick The time tick to archive to.
     * @note We can extend this to pass in a functor or some other object to allow it to handle archival details.
     * @see Object::archive
     */
    unsigned long archive(eint tick = PLUS_INFINITY);


    // PSPlanDatabase methods
    virtual PSList<PSObject*> getAllObjects() const;
    virtual PSList<PSObject*> getObjectsByType(const std::string& objectType) const;
    virtual PSObject* getObjectByKey(PSEntityKey id) const;
    virtual PSObject* getObjectByName(const std::string& name) const;

    virtual PSList<PSToken*> getAllTokens() const;
    virtual PSToken* getTokenByKey(PSEntityKey id) const;

    virtual PSList<PSVariable*> getAllGlobalVariables() const;

    ObjectId createObject(const std::string& objectType,
                            const std::string& objectName,
                            const std::vector<const Domain*>& arguments);

    TokenId createToken(const std::string& tokenType,
                        const std::string& tokenName,
                        bool rejectable=false,
                        bool isFact=false);

    TokenId createSlaveToken(const TokenId master,
                             const std::string& tokenType,
                             const std::string& relation);

    bool hasTokenTypes() const;

    PSPlanDatabaseClient* getPDBClient();

    virtual std::string toString();

  private:
    friend class Token;
    friend class Object;
    friend class PlanDatabaseListener;
    friend class ObjectVariableListener;

    void notifyAdded(const ObjectId object);

    void notifyRemoved(const ObjectId object);

    void notifyAdded(const TokenId token);

    void notifyRemoved(const TokenId token);

    void notifyAdded(const ObjectId object, const TokenId token);

    void notifyRemoved(const ObjectId object, const TokenId token);

    void notifyActivated(const TokenId token);

    void notifyDeactivated(const TokenId token);

    void notifyMerged(const TokenId token);

    void notifySplit(const TokenId token);

    void notifyRejected(const TokenId token);

    void notifyReinstated(const TokenId token);

    void notifyCommitted(const TokenId token);

    void notifyTerminated(const TokenId token);

    void notifyConstrained(const ObjectId object, const TokenId predecessor, const TokenId successor);

    void notifyFreed(const ObjectId object, const TokenId predecessor, const TokenId successor);

    void notifyAdded(const PlanDatabaseListenerId listener);

    void notifyRemoved(const PlanDatabaseListenerId listener);

    /**
     * @brief If an object induces an ordering constraint on a token, it notfifies the plan database.
     * @param object The object on which the token should be ordered.
     * @param token The token to order
     */
    void notifyOrderingRequired(const ObjectId object, const TokenId token);

    /**
     * @brief If an object's state has changed such that a previously required ordering is no longer required then it should
     * notify the plan database.
     */
    void notifyOrderingNoLongerRequired(const ObjectId object, const TokenId token);

    void makeObjectVariable(const std::string& objectType, const std::list<ObjectId>& objects,
			    const ConstrainedVariableId objectVar,
			    bool leaveOpen = false);

    void handleObjectVariableDeletion(const ConstrainedVariableId objectVar);

    void handleObjectVariableCreation(const std::string& objectType,
				      const ConstrainedVariableId objectVar,
				      bool leaveOpen = false);

    /* Useful internal accessors and indexes for accessing objects and tokens in the PlanDatabase. */

    /**
     * @brief Retrieve all objects that can be assigned tokens of a given predicate.
     * @param predicate The predicate name to search by. It must be defined according to the schema.
     * @param results A collection, initially empty, which is an output parameter for results.
     * @see Schema::isPredicateDefined, Schema::canBeAssigned
     */
    void getObjectsByPredicate(const std::string& predicate, std::list<ObjectId>& results);

    /**
     * @brief Utility to index an active token.
     */
    void insertActiveToken(const TokenId token);

    /**
     * @brief Utility to remove an active token
     */
    void removeActiveToken(const TokenId token);

    PlanDatabaseId m_id;
    const ConstraintEngineId m_constraintEngine;
    const SchemaId m_schema;
    TemporalAdvisorId m_temporalAdvisor;
    DbClientId m_client; /*!< A client interface which provides an interception point for transaction logging */
    PSPlanDatabaseClient* m_psClient; // PSEngine wrapper for m_client
    State m_state;
    TokenSet m_tokens;
    ObjectSet m_objects;
    TokenSet m_globalTokens;
    ConstrainedVariableSet m_globalVariables;
    bool m_deleted;
    std::vector<PlanDatabaseListenerId> m_listeners;

    /* In the data structures below, the key is a LabelStr representation of a name */
    std::map<std::string, ObjectId> m_objectsByName; /*!< Object names are unique. Holds all objects m_objectsByName.size() == m_objects.size(). */
    std::multimap<std::string, ObjectId> m_objectsByPredicate; /*!< May be updated every time we add a Token, or remove an object. */
    std::multimap<std::string, ObjectId> m_objectsByType; /*!< May be updated every time we add a Token, or remove an object. */
    std::set<std::string> m_closedObjectTypes; /*!< The set of explicitly closed object types.
					     If present here, it cannot be present in m_objectVariablesByType */
    std::map<std::string, ConstrainedVariableId> m_globalVarsByName;
    std::map<std::string, TokenId> m_globalTokensByName;
    std::map<eint, std::pair<TokenId, ObjectSet> > m_tokensToOrder; /*!< All tokens to order, with the object
								     inducing the requirement stored in the set */

    std::map<std::string, TokenSet > m_activeTokensByPredicate; /*!< All active tokens sorted by predicate */

    // All this to store variables (and their listeners) for Open Object Types
    typedef std::multimap<std::string, std::pair<ConstrainedVariableId, ConstrainedVariableListenerId> > ObjVarsByObjType;
    typedef ObjVarsByObjType::iterator ObjVarsByObjType_I;
    typedef ObjVarsByObjType::const_iterator ObjVarsByObjType_CI;
    ObjVarsByObjType m_objectVariablesByObjectType;
private:
    PlanDatabase(const PlanDatabase&);
    PlanDatabase& operator=(const PlanDatabase&);
  };


template<class ID>
void PlanDatabase::getObjectsByType(const std::string& type, std::list<ID>& results) {
  check_error(results.empty());
  checkError(m_schema->isObjectType(type),
             "Is not an object type in the plan database: " + type);
    
  for (std::multimap<std::string, ObjectId>::const_iterator it = m_objectsByType.find(type);
       it != m_objectsByType.end() && it->first == type; ++it) {
    debugMsg("PlanDatabase:getObjectsByType",
             "Adding object '" << it->second->getName() << "' of type '" <<
             it->second->getType() << "' for type '" << type << "'");
    const Object& o = *(it->second);
    debugMsg("PlanDatabase:getObjectsByType", "Typeid for object: " << typeid(o).name());
    results.push_back(ID(it->second));
  }
}
}

#endif
