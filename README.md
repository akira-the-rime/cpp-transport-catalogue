ENG: 
Transport Catalogue.
This program is designed to practice algorithms (the catalogue is implemented as a directed-weighted graph) and to get accustomed to and adapt to proper software architecture, including infrastructure, business logic, and domain entities. 
It requires the C++20 language standard.
1) To use the program, parse JSON requests into a "Document" variable, using the json::Load() function.
2) Instantiate the catalogue::TransportCatalogue and map_renderer::MapRenderer classes.
3) Instantiate the json_reader::JsonReader class to connect all the previously-created classes and handle the requests within the Document variable, using the HandleRequests() method.
Additionally, I created the json::Builder class to ensure that users cannot pass invalid JSON commands.

RU:
Транспортный каталог.
Эта программа создана для отработки алгоритмов (каталог реализован по средствам ориентированного взвешенного графа) и для того, чтобы привыкнуть и адаптироваться к правильной программной архитектуре, включающую инфраструктуру, бизнес-логику и сущности предметной области.
Для запуска программы требуется стандарт языка C++20.
1) Чтобы использовать программу, распарсите JSON-запросы в переменную типа "Document" с помощью функции json::Load().
2) Инстанцируйте классы catalogue::TransportCatalogue и map_renderer::MapRenderer.
3) Затем создайте объект класса json_reader::JsonReader, чтобы связать ранее созданные классы и обработать запросы из переменной типа "Document" с помощью метода HandleRequests().
В дополнение, я создал класс json::Builder, чтобы пользователь не мог передавать невалидные JSON-команды.
