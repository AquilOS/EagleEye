#pragma once

/*
 *  Nodes are the base processing object of this library.  Nodes can encapsulate all aspects of image processing from
 *  a single operation to a parallel set of intercommunicating processing stacks.  Nodes achieve this through one or more
 *  of the following concepts:
 *
 *  1) Processing node - Input image -> output image
 *  2) Function node - publishes a boost::function object to be used by sibling nodes
 *  3) Object node - publishes an object to be used by sibling nodes
 *  4) Serial collection nodes - Input image gets processed by all child nodes in series
 *  5) Parallel collection nodes - Input image gets processed by all child nodes in parallel. One thread per node
 *
*/

#include <EagleLib.h>
#include <opencv2/core.hpp>
#include <opencv2/cuda.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/thread/future.hpp> 
#include <type_traits>
#include "../RuntimeObjectSystem/RuntimeLinkLibrary.h"
#include "../RuntimeObjectSystem/ObjectInterface.h"


RUNTIME_COMPILER_LINKLIBRARY("-lopencv_core -lopencv_cuda");

//#define ADD_PARAMETER(NAME, CONSTRUCTOR, TYPE, PARAMTYPE) parameters.push_back(boost::shared_ptr<TypedParameter< (TYPE) >(new TypedParameter< (TYPE) >((NAME), (NAME), (CONSTRUCTOR), (PARAMTYPE))));
class CV_EXPORTS IObject
{

};
namespace EagleLib
{
    class CV_EXPORTS Parameter
    {
    public:
        enum ParamType
        {
            None = 0,
            Input = 1,
            Output = 2,     //
            Control = 4,    // User runtime controllable parameter
            Local = 8,
            Shared = 16, // Shared accross all instances of this object
            Global = 32,
            State = 64 // State parameter to be read displayed
        };
        std::string name;
        std::string quickHelp;
        ParamType type;
        bool changed;
    protected:
        Parameter(){}
        Parameter(const std::string& name_, const std::string& quickHelp_): name(name_), quickHelp(quickHelp_), type(None)  {}
        Parameter(const std::string& name_, const std::string& quickHelp_, ParamType type_): name(name_), quickHelp(quickHelp_),type(type_)  {}
        virtual ~Parameter(){}
    };

	template<typename T> void cleanup(T ptr, typename std::enable_if<std::is_pointer<T>::value>::type* = 0) {delete ptr;}
	template<typename T> void cleanup(T ptr, typename std::enable_if<!std::is_pointer<T>::value>::type* = 0){return;}

    // Default typed parameter
    template<typename T>
    class CV_EXPORTS TypedParameter: public Parameter
    {
    public:
        typedef boost::shared_ptr<TypedParameter<T>> Ptr;
        TypedParameter(){}
		~TypedParameter()
		{
			// If we are holding a pointer to an object, delete the pointer	
			cleanup(data);
		}

        TypedParameter(const std::string& name, T data_): Parameter(name," "), data(data_){}
        TypedParameter(const std::string& name, T data_, ParamType type_ ): Parameter(name," "){}
        TypedParameter(const std::string& name, const std::string& quickHelp_): Parameter(name,quickHelp_){}
        TypedParameter(const std::string& name, const std::string& quickHelp_, ParamType type_): Parameter(name,quickHelp_, type_){}
        TypedParameter(const std::string& name, const std::string& quickHelp_, T data_, ParamType type_): Parameter(name,quickHelp_, type_), data(data_){}
        T& get();
        T data;
    };
    class CV_EXPORTS Node: public IObject
    {
    public:
        Node();
        virtual ~Node();
        // Primary call to a node.  For simple operations this is the only thing that matters
        cv::cuda::GpuMat          process(cv::cuda::GpuMat& img);

        virtual cv::cuda::GpuMat doProcess(cv::cuda::GpuMat& img);
        virtual void             doProcess(cv::cuda::GpuMat& img, boost::promise<cv::cuda::GpuMat>& retVal);
        virtual std::string      getName();
        virtual void             getInputs();
        /********************** Display stuff ****************************/
        // Registers a function to call for displaying the results
        virtual void registerDisplayCallback(boost::function<void(cv::Mat)>& f);
        virtual void registerDisplayCallback(boost::function<void(cv::cuda::GpuMat)>& f);
        virtual void spawnDisplay();
        virtual void killDisplay();

        /******************* Children *******************************/
        virtual int  addChild(Node* child);
        virtual int  addChild(boost::shared_ptr<Node> child);
        virtual void removeChild(boost::shared_ptr<Node> child);
        virtual void removeChild(int idx);

		// Parameter stuff
        template<typename T> int addParameter(const std::string& name, T data, const std::string quickHelp = std::string(), Parameter::ParamType type_ = Parameter::Control)
		{
            parameters.push_back(boost::shared_ptr< TypedParameter<T> >(new TypedParameter<T>(name, quickHelp, data, type_)));
			return parameters.size() - 1;
		}

        template<typename T> boost::shared_ptr< TypedParameter<T> > getParameter(std::string name)
        {
            for(int i = 0; i < parameters.size(); ++i)
            {
                if(parameters[i]->name == name)
                    return boost::dynamic_pointer_cast<T, Parameter>(parameters[i]);
            }
            return boost::shared_ptr<T>();
        }

        template<typename T> boost::shared_ptr< TypedParameter<T> > getParameter(int idx)
        {
            return boost::dynamic_pointer_cast<TypedParameter<T>, Parameter>(parameters[idx]);
        }
       

        virtual bool subParameterExists(std::string name)
        {
            for(int i = 0; i < childParameters.size(); ++i)
            {
                if(childParameters[i].second->name == name)
                {
                    return true;
                }
            }
            return false;
        }
        template<typename T> bool checkSubParameterType(std::string name)
        {
            for(int i = 0; i < childParameters.size(); ++i)
            {
                if(childParameters[i].second->name == name)
                {
                    return boost::dynamic_pointer_cast<TypedParameter<T>, Parameter>(childParameters[i]) != NULL;
                }
            }
        }
        template<typename T> boost::shared_ptr< TypedParameter<T> > getSubParameter(std::string name)
        {
            for(int i = 0; i < childParameters.size(); ++i)
            {
                if(childParameters[i].second->name == name)
                {
                    return boost::dynamic_pointer_cast<TypedParameter<T>, Parameter>(childParameters[i].second);
                }
            }
            return boost::shared_ptr< TypedParameter<T> >(); // Return a NULL pointer
        }

        /*!
         *  \brief findInputs recursively finds any compatible inputs wrt the templated desired type.
         *  \brief usage includes finding all output images
         *  \param output is a vector of the output parameters including a list of the names of where they are from
         */
        template<typename T> void
        findInputs(std::vector<std::string>& nodeNames, std::vector< boost::shared_ptr< TypedParameter<T> > >& parameterPtrs, int hops = 10000)
        {
            if(hops < 0)
                return;
            for(int i = 0 ; i < parameters.size(); ++i)
            {
                if(parameters[i]->type & Parameter::Output) // Can't use someone's input or control parameter, that would be naughty
                    if(boost::dynamic_pointer_cast<TypedParameter<T>, Parameter>(parameters[i]))
                    {
                        nodeNames.push_back(treeName);
                        parameterPtrs.push_back(boost::dynamic_pointer_cast<TypedParameter<T>, Parameter>(parameters[i]));
                    }
            }
            // Recursively check children for any available output parameters that match the input signature
            for(int i = 0; i < children.size(); ++i)
               children[i]->findInputs<T>(nodeNames, parameterPtrs, hops - 1);
            return;
        }

        boost::function<void(std::string)>                  errorCallback;
        boost::function<void(std::string)>                  warningCallback;
        boost::function<void(std::string)>                  statusCallback;
        boost::function<int(std::vector<std::string>)>       inputSelector;
        std::vector< boost::shared_ptr<Node> >  children;
        boost::shared_ptr<Node>                 parent;
        std::string                             nodeName;       // Constant name that describes the node ie: Sobel
        std::string                             treeName;       // Name as placed in the tree ie: Sobel1
        // Parameters of this node
        std::vector< boost::shared_ptr< Parameter > > parameters;
        // Parameters of the child, paired with the index of the child
        std::vector< std::pair< int, boost::shared_ptr< Parameter > > > childParameters;

        boost::function<void(cv::Mat)>          cpuCallback;
        boost::function<void(cv::cuda::GpuMat)> gpuCallback;
        bool drawResults;
    };
}





