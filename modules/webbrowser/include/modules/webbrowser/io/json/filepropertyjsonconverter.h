/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <modules/webbrowser/io/json/glmjsonconverter.h>
#include <inviwo/core/properties/fileproperty.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace inviwo {

/**
 * Converts an FileExtension to a JSON object.
 * Produces layout according to the members of FileExtension:
 * {{"extension", extension},
 * {"description", description} }
 * @see FileExtension
 *
 * Usage example:
 * \code{.cpp}
 * FileExtension p;
 * json j = p;
 * \endcode
 */
void to_json(json& j, const FileExtension& p) {
    j = json{{"extension", p.extension_},
        {"description", p.description_}
    };
}

/**
 * Converts a JSON object to an FileExtension.
 * Expects object layout according to the members of FileExtension:
 * {{"extension", extension},
 * {"description", description} }
 * @see FileExtension
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<FileExtension>();
 * \endcode
 */
void from_json(const json& j, FileExtension& p) {
    j["extension"].get_to(p.extension_);
    j["description"].get_to(p.description_);
}

/**
 * Converts an FileProperty to a JSON object.
 * Produces layout according to the members of FileProperty:
 * {{"value", filePath},
 *  {"selectedExtension", FileExtension},
 *  {"acceptMode", AcceptMode},
 *  {"fileMode", FileMode},
 *  {"contentType", ContentType},
 *  {"nameFilters", std::vector<FileExtension>}
 * }
 * @see FileProperty
 *
 * Usage example:
 * \code{.cpp}
 * FileProperty p;
 * json j = p;
 * \endcode
 */
void to_json(json& j, const FileProperty& p) {
    j = json{{"value", p.get()},
        {"selectedExtension", p.getSelectedExtension()},
        {"acceptMode", p.getAcceptMode()},
        {"fileMode", p.getFileMode()},
        {"contentType", p.getContentType()},
        {"nameFilters", p.getNameFilters()}
    };
}

/**
 * Converts a JSON object to an FileProperty.
 * Expects object layout according to the members of FileProperty:
 * {{"value", filePath},
 *  {"selectedExtension", FileExtension},
 *  {"acceptMode", AcceptMode},
 *  {"fileMode", FileMode},
 *  {"contentType", ContentType},
 *  {"nameFilters", std::vector<FileExtension>},
 *  {"requestFile", ""}, // Will call requestFile()
 * }
 * @see FileProperty
 *
 * Usage example:
 * \code{.cpp}
 * auto p = j.get<FileProperty>();
 * \endcode
 */
void from_json(const json& j, FileProperty& p) {
    if (j.count("value") > 0) {
        p.set(j.at("value").get<std::string>());
    }
    if (j.count("selectedExtension") > 0) {
       p.setSelectedExtension(j.at("selectedExtension").get<FileExtension>());
    }
    if (j.count("acceptMode") > 0) {
        p.setAcceptMode(j.at("acceptMode").get<AcceptMode>());
    }
    if (j.count("fileMode") > 0) {
        p.setFileMode(j.at("fileMode").get<FileMode>());
    }
    if (j.count("contentType") > 0) {
        p.setContentType(j.at("contentType").get<std::string>());
    }
    if (j.count("requestFile") > 0) {
        p.requestFile();
    }
}

}  // namespace inviwo

