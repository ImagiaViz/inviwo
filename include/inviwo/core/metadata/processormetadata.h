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
 * Primary author : Timo Ropinski
 *
 **********************************************************************/

#ifndef IVW_PROCESSORMETADATA_H
#define IVW_PROCESSORMETADATA_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/positionmetadata.h>


namespace inviwo {

class IVW_CORE_API ProcessorMetaData : public MetaData {

public:
    ProcessorMetaData();
    ProcessorMetaData(const ProcessorMetaData& rhs);
    ProcessorMetaData& operator=(const ProcessorMetaData& that);
    virtual ~ProcessorMetaData();

    virtual std::string getClassName() const { return "ProcessorMetaData"; }
    virtual ProcessorMetaData* clone() const;

    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);

    void setPosition(ivec2 pos);
    ivec2 getPosition();
    void setVisibile(bool visibility);
    bool isVisible();

private:
    IVec2MetaData positionMetaData_;
    BoolMetaData visiblityMetaData_;
};

} // namespace

#endif // IVW_PROCESSORMETADATA_H
