from pymongo import MongoClient

class AnimalShelter:
    """ CRUD operations for Animal collection in MongoDB """

    def __init__(self, username, password):  # ✅ Accepts credentials
        """Initialize MongoDB connection with authentication."""
        HOST = 'nv-desktop-services.apporto.com'
        PORT = 30286
        DB = 'AAC'
        COL = 'animals'

        # ✅ Use the provided username and password
        self.client = MongoClient(f'mongodb://{username}:{password}@{HOST}:{PORT}')
        self.database = self.client[DB]
        self.collection = self.database[COL]

    def create(self, data):
        """Inserts a document into MongoDB collection."""
        if data and isinstance(data, dict):
            try:
                result = self.collection.insert_one(data)
                return True if result.inserted_id else False
            except Exception as e:
                print(f"Error inserting document: {e}")
                return False
        else:
            print("Error: Data must be a non-empty dictionary.")
            return False

    def read(self, query=None):  # ✅ Allows calling read() without parameters
        """Retrieves documents from MongoDB collection."""
        if query is None:  # ✅ If no query is given, use an empty dictionary
            query = {}

        if isinstance(query, dict):
            try:
                results = list(self.collection.find(query))
                print(f"MongoDB returned {len(results)} documents.")  # ✅ Debugging output
                return results if results else []
            except Exception as e:
                print(f"Error reading documents: {e}")
                return []
        else:
            print("Error: Query must be a dictionary.")
            return []


    def update(self, query, new_values):
        """Updates documents in MongoDB collection."""
        if query and new_values and isinstance(query, dict) and isinstance(new_values, dict):
            try:
                result = self.collection.update_many(query, {"$set": new_values})
                return result.modified_count  # Returns number of updated documents
            except Exception as e:
                print(f"Error updating documents: {e}")
                return 0
        else:
            print("Error: Query and new values must be non-empty dictionaries.")
            return 0

    def delete(self, query):
        """Deletes documents from MongoDB collection."""
        if query and isinstance(query, dict):
            try:
                result = self.collection.delete_many(query)
                return result.deleted_count  # Returns number of deleted documents
            except Exception as e:
                print(f"Error deleting documents: {e}")
                return 0
        else:
            print("Error: Query must be a non-empty dictionary.")
            return 0
