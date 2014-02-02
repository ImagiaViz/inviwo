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

#ifndef IVW_INVIWOAPPLICATION_H
#define IVW_INVIWOAPPLICATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/moduleaction.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/timer.h>


namespace inviwo {

class IVW_CORE_API InviwoApplication : public Singleton<InviwoApplication> {

public:
    typedef void (*registerModuleFuncPtr)(InviwoApplication*);

    InviwoApplication();
    InviwoApplication(std::string displayName, std::string basePath);
    InviwoApplication(int argc, char** argv, std::string displayName, std::string basePath);
    virtual ~InviwoApplication();

    virtual void initialize(registerModuleFuncPtr);
    virtual void deinitialize();
    virtual bool isInitialized() { return initialized_; }

    enum PathType {
        PATH_DATA,      // data/
        PATH_VOLUMES,   // data/volumes/
        PATH_MODULES,   // modules/
        PATH_WORKSPACES,// data/workspaces/
        PATH_IMAGES,     // data/images/
        PATH_RESOURCES,  // resources/
        PATH_TRANSFERFUNCTIONS  // data/transferfunctions/
    };

    virtual void closeInviwoApplication() {LogWarn("this application have not implemented close inviwo function");}

    /**
     * Get the base path of the application.
     *
     * @return
     */
    const std::string& getBasePath() { return basePath_; }

    /**
     * Get basePath +  pathType + suffix.
     * @see PathType
     * @param pathType Enum for type of path
     * @param suffix Path extension
     * @return basePath +  pathType + suffix
     */
    std::string getPath(PathType pathType, const std::string& suffix = "");

    void registerModule(InviwoModule* module) { modules_.push_back(module); }
    const std::vector<InviwoModule*> getModules() const { return modules_; }

    void setProcessorNetwork(ProcessorNetwork* processorNetwork) { processorNetwork_ = processorNetwork; }
    ProcessorNetwork* getProcessorNetwork() { return processorNetwork_; }

    template<class T> T* getSettingsByType();

    const CommandLineParser* getCommandLineParser() const { return commandLineParser_; }
    template<class T> T* getModuleByType();

    virtual void registerFileObserver(FileObserver* fileObserver) { LogWarn("This Inviwo application does not support FileObservers."); }
    virtual void startFileObservation(std::string fileName) { LogWarn("This Inviwo application does not support FileObservers."); }
    virtual void stopFileObservation(std::string fileName) { LogWarn("This Inviwo application does not support FileObservers."); }

    std::string getDisplayName()const {return displayName_;}

    enum MessageType {
        IVW_OK,
        IVW_ERROR
    };
    virtual void playSound(unsigned int soundID) { /*LogWarn("This Inviwo application does not support sound feedback.");*/ }

    /**
     * Creates a timer. Caller is responsible for deleting returned object.
     * @see Timer
     * @return new Timer
     */
    virtual Timer* createTimer() const { LogWarn("This application has not implemented any timer"); return NULL; }

    virtual void addCallbackAction(ModuleCallbackAction* callbackAction);

    virtual std::vector<ModuleCallbackAction*> getCallbackActions();

    std::vector<Settings*> getModuleSettings();

protected:
    void printApplicationInfo();

private:
    std::string displayName_;

    std::string basePath_;

    std::vector<InviwoModule*> modules_;

    ProcessorNetwork* processorNetwork_;

    CommandLineParser* commandLineParser_;

    bool initialized_;

    std::vector<ModuleCallbackAction*> moudleCallbackActions_;
};

template<class T>
T* InviwoApplication::getSettingsByType() {
    T* settings = getTypeFromVector<T>(getModuleSettings());
    return settings;
}

template<class T>
T* InviwoApplication::getModuleByType() {
    T* module = getTypeFromVector<T>(getModules());
    return module;
}

} // namespace

#endif // IVW_INVIWOAPPLICATION_H
