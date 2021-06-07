#include <iostream>
#include <occi.h>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct ShoppingCart {
  int product_id;
  double price;
  int quantity;
};

// A function that just return a true if it gets 1 or 0 as input and false if not
// Using it twice in my code so desided to separated it.
bool getSafeInt(int& result) {
  bool flag = true;
  cin >> result;
  if (cin.fail()) {
    cin.clear();
    cin.ignore(1000);
  }
  else {
    if (result == 0 || result == 1)
      flag = false;
  }
  return flag;
}

int mainMenu();
int customerLogin(Connection* conn, int customerId);
int addToCart(Connection* conn, struct ShoppingCart cart[]);
double findProduct(Connection* conn, int product_id);
void displayProducts(struct ShoppingCart cart[], int products);
int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount);

int main(void) {
  // OCCI VARIABLES
  Environment* env = nullptr;
  Connection* conn = nullptr;
  // USER DATA
  string user = "";   // username
  string pass = "";   // password
  string constr = ""; // conection string

  try {
    // establishing the connection
    env = Environment::createEnvironment(Environment::DEFAULT);
    conn = env->createConnection(user, pass, constr);

    // the main body of a program that connects all parts(functions)
    // continue to run until mainMenu returns 0
    while (mainMenu()) {
      // program asks to enter customer id
      int customerId;
      cout << "Enter the customer ID: ";
      cin >> customerId;
      if (cin.fail()) {
        cin.clear();
        cin.ignore(1000);
      }
      // runs following code if found the customer, skips otherwise
      else if (customerLogin(conn, customerId)) {
        ShoppingCart cart[5];
        int noOfItems;
        noOfItems = addToCart(conn, cart);
        displayProducts(cart, noOfItems);
        checkout(conn, cart, customerId, noOfItems);
      }
      else
        cout << "The customer does not exist." << endl;
    }
    cout << "Good bye!..." << endl;

    env->terminateConnection(conn);
    Environment::terminateEnvironment(env);
  }
  // catches all the SQL error that can be thrown in any of the functions
  catch (SQLException& e) {
    cout << e.getErrorCode() << ": " << e.getMessage();
  }
}

// Displays main menu and asks to choose an option, nothing complicated
int mainMenu() {
  int result;
  bool flag = 0;
  do {
    cout << "******************** Main Menu ********************" << endl;
    cout << "1) Login" << endl;
    cout << "0) Exit" << endl;
    if (flag)
      cout << "You entered a wrong value. ";
    cout << "Enter an option (0-1): ";
    flag = getSafeInt(result);
  } while (flag);
  return result;
}

// Creates the statement by calling the stored procedures
// from the server side and passing IN and OUT arguments
int customerLogin(Connection* conn, int customerId) {
  int found = 0;
  try {
    Statement* stmt = conn->createStatement();

    // sets the SQL statement
    stmt->setSQL("BEGIN find_customer(:1, :2); END;");

    // assigns the params to the procedure
    stmt->setNumber(1, customerId);
    stmt->registerOutParam(2, Type::OCCIINT, sizeof(found));

    // runs the statement
    stmt->executeUpdate();
    found = stmt->getInt(2);

    // terminates the statement
    conn->terminateStatement(stmt);
  } catch (SQLException& e) { // throws SQL errors further if any occured
    throw e;
  }

  return found;
}

// this function fills the shopping cart with items by asking
// product Id and calling function that founds it in the DB
// and than adding it's information to the ShoppingCart array
int addToCart(Connection* conn, ShoppingCart cart[]) {
  bool flag = 0;
  int noOfItems = 0; // this variable stores number of items, but not their total quantity
                     // it is used to identify how many elements of the array are filled
  cout << "------------------ Add Products to Cart ------------------" << endl;
  do {
    int productId;
    do {
      flag = true;

      cout << "Enter the product ID: ";
      cin >> productId;
      if (cin.fail()) {
        cin.clear();
        cin.ignore(1000);
      }
      else {
        try {
          // calls the function to get the price and at the same time identify if product with entered id is in DB
          double productPrice = findProduct(conn, productId);
          if (productPrice) {
            cout << "Product Price: " << productPrice << endl;

            unsigned int productQuantity;
            cout << "Enter the product Quantity: ";
            cin >> productQuantity;

            // fills the element of the cart with data
            cart[noOfItems].product_id = productId;
            cart[noOfItems].price = productPrice;
            cart[noOfItems].quantity = productQuantity;
            noOfItems++;
            flag = false;
          }
        }
        catch (SQLException& e) { // throws SQL errors further if any occured
          throw e;
        }
      }

      if (flag)
        cout << "The product does not exists. Try again..." << endl;
    } while (flag);
    flag = true;

    // asks user to continue or not
    int choice;
    while (flag && noOfItems < 5) {
      cout << "Enter 1 to add more products or 0 to checkout: ";
      flag = getSafeInt(choice);
    }
    if (choice) flag = true;

  } while (flag && noOfItems < 5);
  cout << endl;
  return noOfItems;
}

// this functions finds product with provided id
//and returns its price if product exists, otherwise returns zero
double findProduct(Connection* conn, int product_id) {
  double price = 0;
  try {
    // creates statement that uses stored procedure
    Statement* stmt = conn->createStatement();
    stmt->setSQL("BEGIN find_product(:1, :2); END;");

    // sets IN and OUT params for the procedure
    stmt->setNumber(1, product_id);
    stmt->registerOutParam(2, Type::OCCIDOUBLE, sizeof(price));

    // executes it
    stmt->executeUpdate();
    // stores OUT parameter in variable
    price = stmt->getDouble(2);
    conn->terminateStatement(stmt);
  }
  catch (SQLException& e) { // throws SQL errors further if any occured
    throw e;
  }
  return price;
}

// This function simply recieves the cart and its size
// and than displays the content of it to console
void displayProducts(ShoppingCart cart[], int products) {
  double totalPrice = 0;

  cout << "------- Ordered Products ---------" << endl;
  for (int i = 0; i < products; i++) {
    cout << "---Item " << i + 1 << endl;
    cout << "Product ID: " << cart[i].product_id << endl;
    cout << "Price: " << cart[i].price << endl;
    cout << "Quantity: " << cart[i].quantity << endl;
    totalPrice += cart[i].price * cart[i].quantity;
  }
  cout << "------------------------------------" << endl;
  cout << "Total: " << totalPrice << endl << endl;
}

// This function submits the data that user entered before into the ShoppingCart array
// to the oracle server using stored procedures from there
int checkout(Connection* conn, ShoppingCart cart[], int customerId, int productCount) {
  int orderId = 0;
  char choice;
  bool flag = 1;
  do {
    cout << "Would you like to checkout? (Y/y or N/n) ";
    cin >> choice;
    if (cin.fail()) {
      cin.clear();
      cin.ignore(1000);
    }
    else {
      if (choice == 'Y' || choice == 'y' || choice == 'N' || choice == 'n')
        flag = false;
    }
    if (flag)
      cout << "Wrong input. Try again..." << endl;
  } while (flag);
  if (choice == 'Y' || choice == 'y') {
    try {
      // sets the SQL statement
      Statement* stmt = conn->createStatement();
      stmt->setSQL("BEGIN add_order(:1, :2); END;");

      // sets IN and OUT params for the procedure
      stmt->setInt(1, customerId);
      stmt->registerOutParam(2, Type::OCCIINT, sizeof(orderId));

      // executes it
      stmt->executeUpdate();
      // stores OUT parameter in variable
      orderId = stmt->getInt(2);

      // sets another SQL statement using procedure that receives five IN params
      stmt->setSQL("BEGIN add_order_item(:1, :2, :3, :4, :5); END;");

      // this loop executes the statement for each item in the ShoppingCart array
      // with new values retrieved from the array
      for (int i = 0; i < productCount; i++) {
        stmt->setInt(1, orderId);
        stmt->setInt(2, i);
        stmt->setInt(3, cart[i].product_id);
        stmt->setInt(4, cart[i].quantity);
        stmt->setDouble(5, cart[i].price);
        stmt->executeUpdate();
      }
      conn->terminateStatement(stmt);
    }
    catch (SQLException& e) { // throws SQL errors further if any occured
      throw e;
    }
    cout << "The order is successfully completed." << endl;
  }
  else
    cout << "The order is cancelled." << endl;
  return orderId;
}
