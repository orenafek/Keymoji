import csv
from sys import argv
from sklearn import tree
from os import walk, listdir
from os.path import isfile, join
import emoji
# Daniel & Belle Love Arian Garnde

class ActionUnit(object):
    __au_string__ = ' AU'
    __au_binary_char__ = '_c'
    __au_density_char__ = '_r'

    __au_numbers__ = [1, 2, 4, 5, 6, 7, 9, 10, 12, 14, 15, 17, 20, 23, 25, 26, 28, 45]
    __au_numbers_intensity_ = [1, 2, 4, 5, 6, 7, 9, 10, 12, 14, 15, 17, 20, 23, 25, 26, 45]

    @staticmethod
    def _number_to_string_(number):
        if number < 10:
            return '0' + str(number)

        return str(number)

    @staticmethod
    def __get_keys__(prefix, postfix, numbers):
        return [prefix + ActionUnit._number_to_string_(number) + postfix for number in numbers]

    @staticmethod
    def au_binary_keys():
        return ActionUnit.__get_keys__(ActionUnit.__au_string__,
                                       ActionUnit.__au_binary_char__, ActionUnit.__au_numbers__)

    @staticmethod
    def au_intensity_keys():
        return ActionUnit.__get_keys__(ActionUnit.__au_string__, ActionUnit.__au_density_char__,
                                       ActionUnit.__au_numbers_intensity_)


def read_file_from_csf(file_name):
    with open(file_name, 'rt') as file:
        reader = csv.DictReader(file, delimiter=',')
        rows = []
        for row in reader:
            acc = get_values_from_rows(row, ActionUnit.au_intensity_keys(), False)
            acc += get_values_from_rows(row, ActionUnit.au_binary_keys(), True)
            rows.append(acc)

        if len(rows) == 1:
             return rows[0]
        return rows

def getAllCSVFiles(directory):
    onlyfiles = [f for f in listdir(directory) if isfile(join(directory, f))]
    result = ['{0}/{1}'.format(directory,f) for f in onlyfiles if f.endswith('.csv')]
    return result


def getClassListFromString(class_string):
    return [s for s in class_string.split(', ')]

def read_from_csv(files):
    all_rows = []
    for file_name in files:
        all_rows.append(read_file_from_csf(file_name))

    return all_rows


def get_values_from_rows(row, keys, integer):
    values = []
    for au in keys:
        if integer:
            value = int(row[au])
        else:
            value = float(row[au])
        values.append(value)
    return values

def get_emoji(num):
    if (num == 5):
        return ':cry:'
    else:
        return ':smile:'

def make_classifier(training_data, class_list):
    # 1 = Anger
    # 2 = Disgust
    # 3 = Fear
    # 4 = Happy
    # 5 = Sad
    # 6 = Surprised

    labels = [1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6]

    clf = tree.DecisionTreeClassifier()
    return clf.fit(training_data, class_list)


def classify_picture(classifier, file_name):
    return classifier.predict(read_file_from_csf(file_name))


def main(training_dir,test_file,class_string):
    classifier = make_classifier(read_from_csv(getAllCSVFiles(training_dir)),
        getClassListFromString(class_string))
    result = classify_picture(classifier, test_file)
    print('Test subject is : {0}'.format(result))


if __name__ == "__main__":
    if len(argv) != 4:
        print("Usage: <csv file folder> <class_list> <test file> ")
    else:
        main(argv[1],argv[2],argv[3])
