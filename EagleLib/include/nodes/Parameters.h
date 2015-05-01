#pragma once
#include <string>
#include "../Manager.h"
#include <opencv2/core/persistence.hpp>
#include "../type.h"
#include "../LokiTypeInfo.h"
#include <opencv2/core.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/filesystem.hpp>
namespace EagleLib
{
#define ENUM(value) (int)value, #value
    class CV_EXPORTS EnumParameter
    {
    public:
        EnumParameter()
        {            currentSelection = 0;        }

        void addEnum(int value, const std::string& enumeration)
        {
            enumerations.push_back(enumeration);
            values.push_back(value);
        }
        std::vector<std::string> enumerations;
        std::vector<int>         values;
        int currentSelection;
    };

    class CV_EXPORTS Parameter
    {
    public:
        typedef boost::shared_ptr<Parameter> Ptr;
        virtual void setSource(const std::string& name) = 0;
        virtual void update() = 0;
        virtual bool acceptsInput(const Loki::TypeInfo& type) = 0;
        virtual void Init(cv::FileNode& fs){}

        virtual void Serialize(cv::FileStorage& fs)
        {
            fs << name << "{";
            fs << "ToolTip" << toolTip;
            fs << "TreeName" << treeName;
            fs << "ParamType" << type;
#ifndef _MSC_VER
            fs << "DataType" << type_info::demangle(typeInfo.name());
#else
            fs << "DataType" << typeInfo.name();
#endif
        }

        enum ParamType
        {
            None		= 0,
            Input		= 1,	// Is an object, function, or variable that is used in this node, expected as an input from another node
            Output		= 2,    // Is an object or function that can be used in another node
            Control		= 4,    // User runtime controllable parameter
            Local		= 8,	//
            Shared		= 16,	// Shared accross all instances of this object
            Global		= 32,	//
            State		= 64,	// State parameter to be read and displayed
            NotifyOnRecompile = 128
        };

        std::string name;
        std::string toolTip;
        std::string treeName;
        Loki::TypeInfo typeInfo;
        ParamType	type;
        bool		changed;
        // Used with input / output parameters to list the number of subscribers to an output
        unsigned int subscribers;
        boost::recursive_mutex mtx;
    protected:
        Parameter(const std::string& name_ = "", const ParamType& type_ = None, const std::string toolTip_ = ""): name(name_),type(type_), changed(false), toolTip(toolTip_), subscribers(0){}
        virtual ~Parameter(){}
    };

    template<typename T> void cleanup(T ptr, typename std::enable_if<std::is_pointer<T>::value>::type* = 0) { delete ptr; }
    template<typename T> void cleanup(T ptr, typename std::enable_if<!std::is_pointer<T>::value>::type* = 0){ return; }



    // Default typed parameter
    template<typename T>
    class TypedParameter : public Parameter
    {
    public:
        typedef boost::shared_ptr< TypedParameter<T> > Ptr;
        typedef T ValType;

        virtual void setSource(const std::string& name){}
        virtual void update(){}
        virtual bool acceptsInput(const Loki::TypeInfo& type){ return false;}
        virtual void Serialize(cv::FileStorage &fs);
        virtual void Init(cv::FileNode &fs);
        TypedParameter(const std::string& name_, const T& data_, int type_ = Control, const std::string& toolTip_ = "", bool ownsData_ = false) :
            Parameter(name_, (ParamType)type_, toolTip_), data(data_), ownsData(ownsData_) {
            typeInfo = Loki::TypeInfo(typeid(T));
        }
        ~TypedParameter(){ if (ownsData)cleanup<T>(data); }
        T data;
    private:
        bool ownsData;
    };
    template<typename T> void TypedParameter<T>::Serialize(cv::FileStorage& fs){
    }
    template<typename T> void TypedParameter<T>::Init(cv::FileNode& fs){
    }
#define SERIALIZE_TYPE(type) template<> inline void TypedParameter<type>::Serialize(cv::FileStorage& fs){       \
    Parameter::Serialize(fs);                                                                                   \
    fs << "Data" << data;\
    fs << "}";\
}                                                                                                               \
    template<> inline void TypedParameter<type>::Init(cv::FileNode& fs){                                        \
    cv::FileNode myNode = fs[name];                                                                             \
    myNode["Data"] >> data;                                                                                     \
}


    SERIALIZE_TYPE(double)
    SERIALIZE_TYPE(float)
    SERIALIZE_TYPE(unsigned char)
    SERIALIZE_TYPE(short)
    SERIALIZE_TYPE(unsigned short)
    //SERIALIZE_TYPE(int)

    SERIALIZE_TYPE(bool)

    template<> inline void TypedParameter<boost::filesystem::path>::Serialize(cv::FileStorage& fs){
        Parameter::Serialize(fs);
        fs << "Data" << data.string();
        fs << "}";
    }
        template<> inline void TypedParameter<boost::filesystem::path>::Init(cv::FileNode& fs){
        cv::FileNode myNode = fs[name];
        std::string pathStr = (std::string)myNode["Data"];
        data = boost::filesystem::path(pathStr);
    }

    template<> inline void TypedParameter<unsigned int>::Serialize(cv::FileStorage& fs){
        Parameter::Serialize(fs);
        fs << "Data" << (int)data;
        fs << "}";
    }
        template<> inline void TypedParameter<unsigned int>::Init(cv::FileNode& fs){
        cv::FileNode myNode = fs[name];
        data = (int)myNode["Data"];
    }

    template<> inline void TypedParameter<int>::Serialize(cv::FileStorage& fs){
        Parameter::Serialize(fs);
        fs << "Data" << data;
        fs << "}";
    }
        template<> inline void TypedParameter<int>::Init(cv::FileNode& fs){
        cv::FileNode myNode = fs[name];
        data = (int)myNode["Data"];
    }

    template<> void inline TypedParameter<EnumParameter>::Serialize(cv::FileStorage& fs)
    {
        Parameter::Serialize(fs);
        fs << "Enumerations" << "[:";
        for(int i = 0; i < data.enumerations.size(); ++i)
        {
            fs << data.enumerations[i];
        }
        fs << "]";
        fs << "Values" << "[:";
        for(int i = 0; i < data.values.size(); ++i)
        {
            fs << data.values[i];
        }
        fs << "]";
        fs << "CurrentSelection" << data.currentSelection;
        fs << "}";
    }
    template<> void inline TypedParameter<EnumParameter>::Init(cv::FileNode& fs)
    {
        cv::FileNode myNode = fs[name];
        data.currentSelection = (int)myNode["CurrentSelection"];
    }

    template<typename T>
    class CV_EXPORTS RangedParameter : public TypedParameter<T>
    {
    public:
        typedef typename boost::shared_ptr<RangedParameter<T> > Ptr;
        typedef T ValType;
        RangedParameter(const std::string& name_,
                        const T& data_,
                        const T& maxVal_,
                        const T& minVal_,
                        const Parameter::ParamType& type_ = Parameter::Control,
                        const std::string& toolTip_ = "",
                        bool ownsData_ = false) :
            TypedParameter<T>(name_, data_, type_, toolTip_, ownsData_), maxVal(maxVal_), minVal(minVal_){}
        void setRange(const T& _max, const T& _min){ maxVal = _max; minVal = _min; }
        T maxVal;
        T minVal;
    };
    template<typename T> T* getParameterPtr(EagleLib::Parameter::Ptr parameter);

    template<typename T>
    class CV_EXPORTS InputParameter : public TypedParameter<T*>
    {
    public:
        InputParameter(const std::string& name_, const std::string& toolTip_ = "") :
            TypedParameter<T*>(name_, nullptr, Parameter::Input, toolTip_, false)
        {

        }
        // TODO: TEST THIS
        virtual void setSource(const std::string& name = std::string())
        {
            if(name == sourceTreeName)
                return;
            if (name.size() != 0)
                sourceTreeName = name;
            update();
        }
        virtual void update()
        {
            if(sourceTreeName.size() == 0)
                return;
            auto param = EagleLib::NodeManager::getInstance().getParameter(sourceTreeName);
            if(param == nullptr)
                this->data = nullptr;
            else
                this->data = getParameterPtr<T>(param);
        }
        virtual bool acceptsInput(const Loki::TypeInfo &type)
        {
            return type == Loki::TypeInfo(typeid(T)) || type == Loki::TypeInfo(typeid(T*)) || type == Loki::TypeInfo(typeid(T&));
        }


        // The full parameter name of the source
        std::string sourceTreeName;
    };
    template<typename T> T* getParameterPtr(EagleLib::Parameter::Ptr parameter)
    {
        // Dynamically check what type of parameter this is refering to
        typename EagleLib::TypedParameter<T>::Ptr typedParam = boost::dynamic_pointer_cast<EagleLib::TypedParameter<T>, EagleLib::Parameter>(parameter);
        if (typedParam)
            return &typedParam->data;


        typename EagleLib::TypedParameter<T*>::Ptr typedParamPtr = boost::dynamic_pointer_cast<EagleLib::TypedParameter<T*>, EagleLib::Parameter>(parameter);
        if (typedParamPtr)
            return typedParamPtr->data;


        typename EagleLib::TypedParameter<T&>::Ptr typedParamRef = boost::dynamic_pointer_cast<EagleLib::TypedParameter<T&>, EagleLib::Parameter>(parameter);
        if (typedParamRef)
            return &typedParamRef->data;

        return nullptr;
    }
}