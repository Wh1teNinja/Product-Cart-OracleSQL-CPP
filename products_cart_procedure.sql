SET SERVEROUT ON

-- find_customer --

CREATE OR REPLACE PROCEDURE find_customer(cust_id NUMBER, found OUT NUMBER) AS
BEGIN
    SELECT COUNT(customer_id) INTO found FROM customers
    WHERE customer_id = cust_id;
EXCEPTION
    WHEN TOO_MANY_ROWS THEN
        DBMS_OUTPUT.PUT_LINE('More than one rows found!');
    WHEN NO_DATA_FOUND THEN
        found := 0;
END find_customer;

------------------------------------------------------------------------------

-- find_product --

CREATE OR REPLACE PROCEDURE find_product(prod_id NUMBER, price OUT products.list_price%TYPE) AS
BEGIN
    SELECT list_price INTO price FROM products
    WHERE product_id = prod_id;
EXCEPTION
    WHEN TOO_MANY_ROWS THEN
        DBMS_OUTPUT.PUT_LINE('More than one rows found!');
    WHEN NO_DATA_FOUND THEN
        price := 0;
END find_product;

------------------------------------------------------------------------------

-- add_order --

CREATE OR REPLACE PROCEDURE add_order(customer_id NUMBER, new_order_id OUT NUMBER) AS
BEGIN
    SELECT MAX(order_id) + 1 INTO new_order_id
    FROM orders;
    
    INSERT INTO orders
    VALUES (new_order_id, customer_id, 'Shipped', 56, sysdate);
EXCEPTION
    WHEN TOO_MANY_ROWS THEN
        DBMS_OUTPUT.PUT_LINE('More than one rows found!');
END add_order;



------------------------------------------------------------------------------

-- add_order_item --

CREATE OR REPLACE PROCEDURE add_order_item(
    orderId   order_items.order_id%type,
    itemId    order_items.item_id%type,
    productId order_items.product_id%type,
    quantity  order_items.quantity%type,
    price     order_items.unit_price%type) AS
BEGIN
    INSERT INTO order_items
    VALUES (orderId, itemId, productId, quantity, price);
END add_order_item;