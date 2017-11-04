#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#ifdef _MSC_VER
	#pragma warning( push )  
	#pragma warning( disable : 4150 )
#endif

#include <typeinfo>
#include <unordered_map>
#include <mutex>
#include <stdexcept>
#include <functional>
#include <string>

namespace Util {
    //! Forward declare the base type of the data storage object
    namespace Templates {
		class BaseMap;
	}

    //! Define alias' for the different types of event callbacks that can be defined
	template<typename T> using EventKeyCallback = std::function<void(const std::string&)>;
	template<typename T> using EventValueCallback = std::function<void(const T&)>;
	template<typename T> using EventKeyValueCallback = std::function<void(const std::string&, const T&)>;

    /*
     *      Name: Blackboard 
     *      Author: Mitchell Croft
	 *		Changed: Bricktricker
     *      Created: 08/11/2016
     *      Modified: 03/11/2017
     *      
     *      Purpose:
     *      Provide a structure for a user to store
     *      generic data. 
     *      
     *      Callback functionality implemented to allow 
     *      listening for when specific keyed data is changed. 
     *      
     *      Warning:
     *      Data types stored on the blackboard must have valid
     *      default and copy constructors defined as well as the
     *      assignment operator.
     *      
     *      Only one callback event of each type will be kept for 
     *      each key of every value type. 
    **/
    class Blackboard {
		public:
        /*----------Singleton Values----------*/
        Blackboard() = default;

		~Blackboard() {
			//Lock the data values
			std::lock_guard<std::mutex> guard(mDataLock);

			//Delete all Value Map values
			for (auto& pair : mDataStorage) {
				//delete pair.second;
			}

		};

		private:
        /*----------Variables----------*/

        //! Store a map of all of the different value types
		std::unordered_map<size_t, std::unique_ptr<Templates::BaseMap>> mDataStorage;

        //! Store a mutex for locking data when in use
		std::mutex mDataLock;

        //! Convert a template type into a unique ID value
        template<typename T> inline size_t templateToID() const;

        //! Ensure that a ValueMap objects exists for a specific type
        template<typename T> inline size_t supportTypeRead(); //throws an exeption, if their is no map of type T
		template<typename T> inline size_t supportTypeWrite();

    public:

        //! Data reading/writing
        template<typename T> void write(const std::string& pKey, const T& pValue, bool pRaiseCallbacks = true);
        template<typename T> const T& read(const std::string& pKey);
        template<typename T> void wipeTypeKey(const std::string& pKey);
        /*----------------*/ void wipeKey(const std::string& pKey);
        /*----------------*/ void wipeBoard(bool pWipeCallbacks = false);

        //! Callback functions
        template<typename T> void subscribe(const std::string& pKey, EventKeyCallback<T> pCb);
        template<typename T> void subscribe(const std::string& pKey, EventValueCallback<T> pCb);
        template<typename T> void subscribe(const std::string& pKey, EventKeyValueCallback<T> pCb);
        template<typename T> void unsubscribe(const std::string& pKey);
        /*----------------*/ void unsubscribeAll(const std::string& pKey);

    };

    namespace Templates {
        /*
         *      Name: BaseMap
         *      Author: Mitchell Croft
         *      Created: 08/11/2016
         *      Modified: 08/11/2016
         *      
         *      Purpose:
         *      Provide a base point for the templated ValueMap
         *      objects to inherit from. This allows the 
         *      blackboard to store pointers to the templated 
         *      versions for storing data.
        **/
        class BaseMap { 
        protected:
            //! Set the Value map to be a friend of the blackboard to allow for construction/destruction of the object
            friend class Blackboard;

            //! Provide virtual methods for wiping keyed information
            inline virtual void wipeKey(const std::string& pKey) = 0;
            inline virtual void wipeAll() = 0;
            inline virtual void unsubscribe(const std::string& pKey) = 0;
            inline virtual void clearAllEvents() = 0;

		public:
			//! need to be public to work with unique_ptr
			BaseMap() = default;
			virtual ~BaseMap() = 0;
        };

        //! Define the default destructor for the BaseMap's pure virtual destructor
		inline BaseMap::~BaseMap() = default;

        /*
         *      Name: ValueMap (General)
         *      Author: Mitchell Croft
         *      Created: 08/11/2016
         *      Modified: 08/11/2016
         *      
         *      Purpose:
         *      Store templated data type information for recollection
         *      and use within the Blackboard singleton object
        **/
        template<typename T>
        class ValueMap : BaseMap {
        protected:
            //! Set the Value map to be a friend of the blackboard to allow for construction/destruction of the object
            friend class Blackboard;

            /*----------Variables----------*/

            //! Store a map of the values for this type
            std::unordered_map<std::string, T> mValues;

            //! Store maps for the callback events 
            std::unordered_map<std::string, EventKeyCallback<T>> mKeyEvents;
            std::unordered_map<std::string, EventValueCallback<T>> mValueEvents;
            std::unordered_map<std::string, EventKeyValueCallback<T>> mPairEvents;

            /*----------Functions----------*/

            //! Privatise the constructor/destructor to prevent external use
            ValueMap() = default;
            ~ValueMap() override {}

            //! Override the functions used to remove keyed information
            inline void wipeKey(const std::string& pKey) override;
            inline void wipeAll() override;
            inline void unsubscribe(const std::string& pKey) override;
            inline void clearAllEvents() override;
        };
    }

    #pragma region Template Definitions
    #pragma region Blackboard
    /*
        Blackboard : templateToID<T> - Convert the template type T to a unique hash code
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type 

        return size_t - Returns the ID as a size_t value
    */
    template<typename T>
    inline size_t Util::Blackboard::templateToID() const {
        //Get the type info
        const std::type_info& type = typeid(T);

        //Return the hash code
        return type.hash_code();
    }

    /*
        Blackboard : supportTypeWrite<T> - Using the type of the template ensure that there is a Value map to support 
                                   holding data of its type
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 04/11/2017

        template T - A generic, non void type

        return size_t - Returns the unique hash code for the template type T
    */
    template<typename T>
    inline size_t Util::Blackboard::supportTypeWrite() {
        //Get the hash code for the type
        size_t key = templateToID<T>();

        //If there isn't a entry for the hash code create a new map
		if (mDataStorage.find(key) == mDataStorage.end()) {
			mDataStorage[key] = std::unique_ptr<Util::Templates::BaseMap>(new Util::Templates::ValueMap<T>());
		}

        //Return the key
        return key;
    }

	/*
	Blackboard : supportTypeRead<T> - Using the type of the template ensure that there is a Value map to support
					holding data of its type. Throws a invalid_argument execption if the Value map could not be found
	Author: Bricktricker
	Created: 04/11/2017

	template T - A generic, non void type

	return size_t - Returns the unique hash code for the template type T
	*/
	template<typename T>
	inline size_t Util::Blackboard::supportTypeRead() {
		//Get the hash code for the type
		size_t key = templateToID<T>();

		//If there isn't a entry for the hash code create a new map
		if (mDataStorage.find(key) == mDataStorage.end()) {
			throw std::invalid_argument("Template not found in Blackboard");
		}

		//Return the key
		return key;
	}
    
    /*
        Blackboard : write<T> - Write a data value to the Blackboard 
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type

        param[in] pKey - The key value to save the data value at
        param[in] pValue - The data value to be saved to the key location
        param[in] pRaiseCallbacks - A flag to indicate if callback events should be raised (Default true)
    */
    template<typename T>
    inline void Util::Blackboard::write(const std::string& pKey, const T& pValue, bool pRaiseCallbacks) {

        //Lock the data
        std::lock_guard<std::mutex> guard(mDataLock);

        //Ensure the key for this type is supported
        size_t key = supportTypeWrite<T>();

        //Cast the Value Map to the type of T
		Util::Templates::ValueMap<T>* map = static_cast<Util::Templates::ValueMap<T>*>(mDataStorage[key].get());

        //Copy the data value across
        map->mValues[pKey] = pValue;

        //Check event flag
        if (pRaiseCallbacks) {
            //Check for events to raise
            if (map->mKeyEvents.find(pKey) != map->mKeyEvents.end() && map->mKeyEvents[pKey]) map->mKeyEvents[pKey](pKey);
            if (map->mValueEvents.find(pKey) != map->mValueEvents.end() && map->mValueEvents[pKey]) map->mValueEvents[pKey](map->mValues[pKey]);
            if (map->mPairEvents.find(pKey) != map->mPairEvents.end() && map->mPairEvents[pKey]) map->mPairEvents[pKey](pKey, map->mValues[pKey]);
        }
    }

    /*
        Blackboard : read<T> - Read the value of a key value from the Blackboard
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type

        param[in] pKey - The key value to read the data value of

        return const T& - Returns a constant reference to the data type of type T
    */
    template<typename T>
    inline const T& Util::Blackboard::read(const std::string& pKey) {

        //Lock the data
        std::lock_guard<std::mutex> guard(mDataLock);

        //Ensure the key for this type is supported
        size_t key = supportTypeRead<T>();

        //Cast the Value Map to the type of T
		Util::Templates::ValueMap<T>* map = static_cast<Util::Templates::ValueMap<T>*>(mDataStorage[key].get());

        //Return the value at the key location
        return map->mValues[pKey];
    }

    /*
        Blackboard : wipeTypeKey - Wipe the value stored at a specific key for the specified type
        Author: Mitchell Croft
        Created: 09/11/2016
        Modified: 09/11/2016

        param[in] pKey - A string object containing the key of the value(s) to remove
    */
    template<typename T>
    inline void Util::Blackboard::wipeTypeKey(const std::string& pKey) {

        //Lock the data
        std::lock_guard<std::mutex> guard(mDataLock);

        //Ensure the key for this type is supported
        size_t key = supportTypeRead<T>();

        //Cast the Value Map to the type of T
		Util::Templates::ValueMap<T>* map = static_cast<Util::Templates::ValueMap<T>*>(mDataStorage[key].get());

        //Wipe the key from the value map
        map->wipeKey(pKey);
    }

    /*
        Blackboard : subscribe<T> - Set the callback event for a specific key value on a type of data
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type

        param[in] pKey - The key to assign the callback event to
        param[in] pCb - A function pointer that takes in a constant string reference as its only parameter
    */
    template<typename T>
    inline void Util::Blackboard::subscribe(const std::string& pKey, EventKeyCallback<T> pCb) {

        //Lock the data
        std::lock_guard<std::mutex> guard(mDataLock);

        //Ensure the key for this type is supported
        size_t key = supportTypeWrite<T>();

        //Cast the Value Map to the type of T
        Util::Templates::ValueMap<T>* map = static_cast<Util::Templates::ValueMap<T>*>(mDataStorage[key].get());

        //Set the event callback
        map->mKeyEvents[pKey] = pCb;
    }

    /*
        Blackboard : subscribe<T> - Set the callback event for a specific key value on a type of data
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type

        param[in] pKey - The key to assign the callback event to
        param[in] pCb - A function pointer that takes in a constant reference to the new value that was assigned
                        as its only parameters
    */
    template<typename T>
    inline void Util::Blackboard::subscribe(const std::string& pKey, EventValueCallback<T> pCb) {

        //Lock the data
        std::lock_guard<std::mutex> guard(mDataLock);

        //Ensure the key for this type is supported
        size_t key = supportTypeWrite<T>();

        //Cast the Value Map to the type of T
		Util::Templates::ValueMap<T>* map = static_cast<Util::Templates::ValueMap<T>*>(mDataStorage[key].get());

        //Set the event callback
        map->mValueEvents[pKey] = pCb;
    }

    /*
        Blackboard : subscribe<T> - Set the callback event for a specific key value on a type of data
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type 

        param[in] pKey - The key to assign the callback event to
        param[in] pCb - A function pointer that takes in a constant string reference and a constant reference to
                        the new value as its only parameters
    */
    template<typename T>
    inline void Util::Blackboard::subscribe(const std::string& pKey, EventKeyValueCallback<T> pCb) {

        //Lock the data
        std::lock_guard<std::mutex> guard(mDataLock);

        //Ensure the key for this type is supported
        size_t key = supportTypeWrite<T>();

        //Cast the Value Map to the type of T
		Util::Templates::ValueMap<T>* map = static_cast<Util::Templates::ValueMap<T>*>(mDataStorage[key].get());

        //Set the event callback
        map->mPairEvents[pKey] = pCb;
    }

    /*
        Blackboard : unsubscribe - Unsubscribe all events associated with a key value
                                   for a specific type
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        param[in] pKey - The key to remove the callback events from
    */
    template<typename T>
    inline void Util::Blackboard::unsubscribe(const std::string& pKey) {

        //Lock the data
        std::lock_guard<std::mutex> guard(mDataLock);

        //Ensure the key for this type is supported
        size_t key = supportTypeRead<T>();

        //Cast the Value Map to the type of T
        Util::Templates::ValueMap<T>* map = static_cast<Util::Templates::ValueMap<T>*>(mDataStorage[key].get());

        //Pass the unsubscribe key to the Value Map
        map->unsubscribe(pKey);
    }
    #pragma endregion

    #pragma region ValueMap
    /*
        ValueMap<T> : wipeKey - Clear the value associated with a key value
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type

        param[in] pKey - The key value to clear the entry of
    */
    template<typename T>
    inline void Util::Templates::ValueMap<T>::wipeKey(const std::string& pKey) {
		mValues.erase(pKey);
	}

    /*
        ValueMap<T> : wipeAll - Erase all data stored in the map
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type
    */
    template<typename T>
    inline void Util::Templates::ValueMap<T>::wipeAll() {
		mValues.clear();
	}

    /*
        ValueMap<T> : unsubscribe - Remove all callback events associated with a key value
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type

        param[in] pKey - The key to wipe all callback events associated with
    */
    template<typename T>
    inline void Util::Templates::ValueMap<T>::unsubscribe(const std::string& pKey) {
        //Find iterators
        auto key = mKeyEvents.find(pKey);
        auto val = mValueEvents.find(pKey);
        auto pair = mPairEvents.find(pKey);

        //Remove the callbacks
        if (key != mKeyEvents.end()) mKeyEvents.erase(key);
        if (val != mValueEvents.end()) mValueEvents.erase(val);
        if (pair != mPairEvents.end()) mPairEvents.erase(pair);
    }

    /*
        ValueMap<T> : clearAllEvents - Clear all event callbacks stored within the Value map
        Author: Mitchell Croft
        Created: 08/11/2016
        Modified: 08/11/2016

        template T - A generic, non void type
    */
    template<typename T>
    inline void Util::Templates::ValueMap<T>::clearAllEvents() {
        //Clear all event maps
        mKeyEvents.clear();
        mValueEvents.clear();
        mPairEvents.clear();
    }
    #pragma endregion
    #pragma endregion
}

/*
    Blackboard : wipeKey - Clear all data associated with the passed in key value
    Author: Mitchell Croft
    Created: 08/11/2016
    Modified: 08/11/2016

    param[in] pKey - A string object containing the key of the value(s) to remove
*/
void Util::Blackboard::wipeKey(const std::string& pKey) {

    //Lock the data
    std::lock_guard<std::mutex> guard(mDataLock);

    //Loop through the different type collections
    for (auto& pair : mDataStorage)
        pair.second->wipeKey(pKey);
}

/*
    Blackboard : wipeBoard - Clear all data stored on the Blackboard
    Author: Mitchell Croft
    Created: 08/11/2016
    Modified: 08/11/2016

    param[in] pWipeCallbacks - Flags if all of the set event callbacks should be cleared
                               as well as the values (Default false)
*/
void Util::Blackboard::wipeBoard(bool pWipeCallbacks) {

    //Lock the data
    std::lock_guard<std::mutex> guard(mDataLock);

    //Loop through all stored Value maps
    for (auto& pair : mDataStorage) {
        //Clear the data values
        pair.second->wipeAll();

        //Clear the callbacks
        if (pWipeCallbacks) pair.second->clearAllEvents();
    }
}

/*
    Blackboard : unsubscribeAll - Remove the associated callback events for a key
                                  from every type map 
    Author: Mitchell Croft
    Created: 08/11/2016
    Modified: 08/11/2016

    param[in] pKey - The key to remove the callback events from
*/
void Util::Blackboard::unsubscribeAll(const std::string& pKey) {

    //Lock the data
    std::lock_guard<std::mutex> guard(mDataLock);

    //Loop through all stored Value maps
    for (auto& pair : mDataStorage)
        pair.second->unsubscribe(pKey);
}

//restore all wanings
#ifdef _MSC_VER
	#pragma warning( pop ) 
#endif

#endif
