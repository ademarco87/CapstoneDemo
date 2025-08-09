from pymongo import MongoClient
import json
import os

class AnimalShelter:
    """CRUD operations for Animal collection in MongoDB or fallback to mock JSON"""

    def __init__(self, username=None, password=None):
        """Initialize MongoDB connection with fallback to mock data."""
        self.use_mock_data = False
        self.mock_data = []

        HOST = "localhost"
        PORT = 4200
        DB = "AAC"
        COL = "animals"

        try:
            if username and password:
                self.client = MongoClient(
                    f"mongodb://{username}:{password}@{HOST}:{PORT}",
                    serverSelectionTimeoutMS=1000,
                    socketTimeoutMS=1000,
                    connectTimeoutMS=1000
                )
            else:
                self.client = MongoClient(
                    f"mongodb://{HOST}:{PORT}",
                    serverSelectionTimeoutMS=1000,
                    socketTimeoutMS=1000,
                    connectTimeoutMS=1000
                )

            self.database = self.client[DB]
            self.collection = self.database[COL]
            # Test the connection
            self.collection.find_one()
            print("Connected to local MongoDB.")
        except Exception as e:
            print(f"[WARN] Could not connect to MongoDB: {e}")
            print("[INFO] Falling back to mock data...")
            self.use_mock_data = True
            self._load_mock_data()

    def _load_mock_data(self):
        """Load data from local JSON file for fallback."""
        try:
            with open("mock_animals.json", "r") as f:
                self.mock_data = json.load(f)
            print(f"[INFO] Loaded {len(self.mock_data)} mock records.")
        except Exception as e:
            print(f"[ERROR] Failed to load mock data: {e}")
            self.mock_data = []

    def create(self, data):
        if self.use_mock_data:
            print("[MOCK MODE] Create operation is disabled.")
            return False

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

    def read(self, query=None):
        if query is None:
            query = {}

        if not isinstance(query, dict):
            print("Error: Query must be a dictionary.")
            return []

        if self.use_mock_data:
            filtered = []
            for doc in self.mock_data:
                match = True
                for key, condition in query.items():
                    doc_value = doc.get(key, None)

                    # Handle Mongo-style conditions
                    if isinstance(condition, dict):
                        if "$in" in condition:
                            if doc_value not in condition["$in"]:
                                match = False
                                break
                        if "$gte" in condition:
                            if not isinstance(doc_value, (int, float)):
                                match = False
                                break
                            if doc_value < condition["$gte"]:
                                match = False
                                break
                        if "$lte" in condition:
                            if not isinstance(doc_value, (int, float)):
                                match = False
                                break
                            if doc_value > condition["$lte"]:
                                match = False
                                break
                    else:
                        # Basic equality check (case-insensitive)
                        if str(doc_value).lower() != str(condition).lower():
                            match = False
                            break

                if match:
                    filtered.append(doc)
            print(f"[MOCK MODE] Returning {len(filtered)} documents.")
            return filtered

        try:
            results = list(self.collection.find(query))
            print(f"MongoDB returned {len(results)} documents.")
            return results if results else []
        except Exception as e:
            print(f"Error reading documents: {e}")
            return []

    def update(self, query, new_values):
        if self.use_mock_data:
            print("[MOCK MODE] Update operation is disabled.")
            return 0

        if query and new_values and isinstance(query, dict) and isinstance(new_values, dict):
            try:
                result = self.collection.update_many(query, {"$set": new_values})
                return result.modified_count
            except Exception as e:
                print(f"Error updating documents: {e}")
                return 0
        else:
            print("Error: Query and new values must be non-empty dictionaries.")
            return 0

    def delete(self, query):
        if self.use_mock_data:
            print("[MOCK MODE] Delete operation is disabled.")
            return 0

        if query and isinstance(query, dict):
            try:
                result = self.collection.delete_many(query)
                return result.deleted_count
            except Exception as e:
                print(f"Error deleting documents: {e}")
                return 0
        else:
            print("Error: Query must be a non-empty dictionary.")
            return 0
