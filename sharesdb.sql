drop table if exists OPENTB, CANCELED, EXECUTED, POSITIONTB, ACCOUNT;

CREATE TABLE ACCOUNT(
       ACCOUNTID INT PRIMARY KEY,
       BALANCE float NOT NULL
);
CREATE TABLE OPENTB(
       ID INT,
       SYMBOL TEXT NOT NULL,
       AMOUNT float NOT NULL,
       PRICE float NOT NULL,
       ACCOUNTID INT references ACCOUNT(ACCOUNTID),
       PRIMARY KEY(ID)
);

CREATE TABLE CANCELED(
       ID INT,
       SYMBOL TEXT NOT NULL,
       AMOUNT float NOT NULL,
       PRICE float NOT NULL,
       EPOCH BIGINT NOT NULL,
       ACCOUNTID INT references ACCOUNT(ACCOUNTID),
       PRIMARY KEY(ID)
);

CREATE TABLE EXECUTED(
       EXECUTEDID SERIAL,
       ID INT,
       SYMBOL TEXT NOT NULL,
       AMOUNT float NOT NULL,
       PRICE float NOT NULL,
       EPOCH BIGINT NOT NULL,
       ACCOUNTID INT references ACCOUNT(ACCOUNTID),
       PRIMARY KEY(EXECUTEDID)
);

CREATE TABLE POSITIONTB(
       ID SERIAL,
       SYMBOL TEXT NOT NULL,
       AMOUNT float NOT NULL,
       ACCOUNTID INT references ACCOUNT(ACCOUNTID),
       PRIMARY KEY(ID)
);