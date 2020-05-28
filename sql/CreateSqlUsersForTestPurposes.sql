CREATE USER IF NOT EXISTS 'supla'@'localhost';
CREATE USER IF NOT EXISTS 'supla'@'%';

GRANT ALL PRIVILEGES ON *.* TO 'supla'@'localhost';
GRANT ALL PRIVILEGES ON *.* TO 'supla'@'%';

FLUSH PRIVILEGES;
