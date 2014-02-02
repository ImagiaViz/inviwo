/**********************************************************************
 * Copyright (C) 2012-2013 Scientific Visualization Group - Link�ping University
 * All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * No part of this software may be reproduced or transmitted in any
 * form or by any means including photocopying or recording without
 * written permission of the copyright owner.
 *
 * Primary author : Sathish Kottravel
 *
 **********************************************************************/

#ifndef IVW_SERIALIZE_BASE_H
#define IVW_SERIALIZE_BASE_H

#ifndef TIXML_USE_TICPP
#  define TIXML_USE_TICPP
#endif

#include <ticpp/ticpp.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/io/serialization/ivwserializeconstants.h>
#include <inviwo/core/util/factory.h>
#include <map>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// include glm
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/core/type.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::dvec2 dvec2;
typedef glm::dvec3 dvec3;
typedef glm::dvec4 dvec4;
typedef glm::bvec2 bvec2;
typedef glm::bvec3 bvec3;
typedef glm::bvec4 bvec4;
typedef glm::uvec2 uvec2;
typedef glm::uvec3 uvec3;
typedef glm::uvec4 uvec4;
typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;
typedef glm::quat quat;

namespace inviwo {

typedef ticpp::Document TxDocument;
typedef ticpp::Element TxElement;
typedef ticpp::Node TxNode;
typedef ticpp::Exception TxException;
typedef ticpp::Declaration TxDeclaration;
typedef ticpp::Comment TxComment;
typedef ticpp::Attribute TxAttribute;
typedef ticpp::Iterator<TxElement> TxEIt;
typedef ticpp::Iterator<TxAttribute> TxAIt;

class IvwSerializable;

class IVW_CORE_API IvwSerializeBase {
public:
    /**
     * \brief Base class for IvwSerializer and IvwDeserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     *
     * @param IvwSerializeBase & s object of similar type.
     * @param bool allowReference disables or enables reference management schemes.
     */
    IvwSerializeBase(IvwSerializeBase& s, bool allowReference=true);
    /**
     * \brief Base class for IvwSerializer and IvwDeserializer.
     *
     * This class consists of features that are common to both serializer
     * and de-serializer. Some of them are reference data manager,
     * (ticpp::Node) node switch and factory registration.
     *
     * @param std::string fileName full path to xml file (for reading or writing).
     * @param bool allowReference disables or enables reference management schemes.
     */
    IvwSerializeBase(std::string fileName, bool allowReference=true);
    /**
     * \brief Destructor
     */
    virtual ~IvwSerializeBase();
    /**
     * \brief gets the xml file name.
     */
    virtual std::string getFileName();
    /**
     * \brief Checks whether the given type is a primitive type.
     *
     * return true if type is one of following type:
     * bool, char, signed int, unsigned int, float, double, long double, std::string
     *
     * @param const std::type_info & type can be one of  bool, char, signed int, unsigned int, float, double, long double, std::string
     * @return bool true or false
     */
    bool isPrimitiveType(const std::type_info& type) const;
    /**
     * \brief Checks whether the given type is a primitive pointer type.
     *
     * return true if type is one of following type:
     * bool*, char*, signed int*, unsigned int*, float*, double*, long double*, std::string*
     *
     * @param const std::type_info & type can be one of  bool, char, signed int, unsigned int, float, double, long double, std::string.
     * @return bool true or false.
     */
    bool isPrimitivePointerType(const std::type_info& type) const;
    /**
     * \brief Enable or disable reference flag.
     */
    void setAllowReference(const bool& allowReference);
    /**
     * \brief Registers all factories from all modules.
     */
    virtual void registerFactories(void);

    /**
     * \brief For allocating objects such as processors, properties.. using registered factories.
     *
     * @param const std::string & className is used by registered factories to allocate the required object.
     * @return T* NULL if allocation fails or className does not exist in any factories.
     */
    template <typename T>
    T* getRegisteredType(const std::string& className);

    /**
     * \brief For allocating objects that do not belong to any registered factories.
     *
     * @return T* Pointer to object of type T.
     */
    template <typename T>
    T* getNonRegisteredType();

    /**
     * \brief To set file name with full path which will be later serialized or deserialzied
     */
    virtual void setFileName(const std::string fileName);

    class IVW_CORE_API NodeSwitch {
    public:
        /**
         * \brief NodeSwitch helps track parent node during recursive/nested function calls.
         *
         * @param IvwSerializeBase & serializer reference to serializer or deserializer
         * @param TxElement * node //Parent (Ticpp Node) element.
         */
        NodeSwitch(IvwSerializeBase& serializer, TxElement* node, bool getChild = true);
        /**
         * \brief Destructor
         */
        ~NodeSwitch();

    private:
        IvwSerializeBase& serializer_;  //reference to serializer or deserializer
        TxElement* storedNode_; //Parent (Ticpp Node) element.
        bool storedGetChild_;
    };

    struct IVW_CORE_API ReferenceData {
        TxElement* node_; //Ticpp Node element.
        bool isPointer_; //Used to differentiate pointer and object.
    };

    typedef std::pair<const void*, IvwSerializeBase::ReferenceData> RefDataPair;
    typedef std::multimap<const void*,ReferenceData> RefMap;
    typedef std::vector<IvwSerializeBase::ReferenceData> RefDataList;

    class IVW_CORE_API ReferenceDataContainer {
    public:
        ReferenceDataContainer();
        ~ReferenceDataContainer();
        size_t insert(const void* data, TxElement* node, bool isPointer=true);
        size_t find(const void* data);
        void* find(const std::string& key, const std::string& reference_or_id);
        TxElement* nodeCopy(const void* data);
        void setReferenceAttributes();

    private:
        RefMap referenceMap_;
        int referenceCount_;
    };

protected:

    static std::string nodeToString(const TxElement& node);

    friend class NodeSwitch;

    std::vector<Factory*> registeredFactories_;
    std::string fileName_;
    TxDocument doc_;
    TxElement* rootElement_;
    bool allowRef_;
    bool getChild_;
    ReferenceDataContainer refDataContainer_;
};


template <typename T>
T* IvwSerializeBase::getRegisteredType(const std::string& className) {
    T* data = 0;
    std::vector<Factory*>::iterator it;

    for (it = registeredFactories_.begin(); it!=registeredFactories_.end(); it++) {
        data = dynamic_cast<T*>((*it)->create(className));

        if (data)
            break;
    }

    return data;
}

template <typename T>
inline T* IvwSerializeBase::getNonRegisteredType() {
    return new T();
}

} //namespace
#endif