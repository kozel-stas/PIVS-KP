extern "C" {
#include "sc_memory_headers.h"
#include "sc_helper.h"
#include "utils.h"
}

#include <stdio.h>
#include <iostream>
#include <glib.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <algorithm>

using namespace std;

sc_memory_context *context;

sc_addr graph, rrel_arcs, rrel_nodes, visit, curr_vertex, father;//зхзхзхз
sc_bool helpme=SC_FALSE;//показывает нужно ли восстанавливать дуги
//из названия все понятно))
sc_bool set_is_not_empty(sc_addr set)
{
    sc_iterator3 *check = sc_iterator3_f_a_a_new(context,
                          set,
                          sc_type_arc_pos_const_perm,
                          0);
    if (SC_TRUE == sc_iterator3_next(check)) {
        return SC_TRUE;
    } else {

        return SC_FALSE;
    }
}
//из названия все понятно))
sc_bool find_vertex_in_set(sc_addr element, sc_addr set)
{
    sc_bool find = SC_FALSE;

    sc_iterator3 *location = sc_iterator3_f_a_a_new(context,
                             set,
                             sc_type_arc_pos_const_perm,
                             0);

    while (SC_TRUE == sc_iterator3_next(location)) {
        sc_addr loc = sc_iterator3_value(location, 2);

        if (SC_ADDR_IS_NOT_EQUAL(loc, element)) {
            find = SC_FALSE;
            continue;
        } else {
            find = SC_TRUE;
            break;
        }
    }
    return find;
}
//из названия все понятно))
void get_edge_vertexes(sc_addr edge, sc_addr &v1, sc_addr &v2)
{
    sc_memory_get_arc_begin(context, edge, &v1);
    sc_memory_get_arc_end(context, edge, &v2);
}
//рисуем граф
void print_graph()
{
    sc_addr arcs, nodes, loc, v1, v2, printed_vertex;
    sc_bool find;
    printed_vertex = sc_memory_node_new(context, sc_type_const);

    printEl(context, graph);
    cout << endl << "----------------------" << endl;

    sc_iterator5 *it = sc_iterator5_f_a_a_a_f_new(context,
                       graph,
                       sc_type_arc_pos_const_perm,
                       0,
                       sc_type_arc_pos_const_perm,
                       rrel_arcs);

    if (SC_TRUE == sc_iterator5_next(it)) {
        arcs = sc_iterator5_value(it, 2);

        sc_iterator3 *arcs_it = sc_iterator3_f_a_a_new(context,
                                arcs,
                                sc_type_arc_pos_const_perm,
                                0);

        while (SC_TRUE == sc_iterator3_next(arcs_it)) {

            sc_addr t_arc = sc_iterator3_value(arcs_it, 2);

            get_edge_vertexes(t_arc, v1, v2);

            printEl(context, v1);
            printf(" -- ");
            printEl(context, v2);
            cout << endl;

            if (SC_FALSE == find_vertex_in_set(v1, printed_vertex))
                sc_memory_arc_new(context, sc_type_arc_pos_const_perm, printed_vertex, v1);
            if (SC_FALSE == find_vertex_in_set(v2, printed_vertex))
                sc_memory_arc_new(context, sc_type_arc_pos_const_perm, printed_vertex, v2);
        }
        sc_iterator3_free(arcs_it);
    }

    sc_iterator5_free(it);

    it = sc_iterator5_f_a_a_a_f_new(context,
                                    graph,
                                    sc_type_arc_pos_const_perm,
                                    0,
                                    sc_type_arc_pos_const_perm,
                                    rrel_nodes);

    if (SC_TRUE == sc_iterator5_next(it)) {
        nodes = sc_iterator5_value(it, 2);

        sc_iterator3 *nodes_it = sc_iterator3_f_a_a_new(context,
                                 nodes,
                                 sc_type_arc_pos_const_perm,
                                 0);


        while (SC_TRUE == sc_iterator3_next(nodes_it)) {

            sc_addr t_node = sc_iterator3_value(nodes_it, 2);

            find = find_vertex_in_set(t_node, printed_vertex);

            if (find == SC_FALSE) {
                printEl(context, t_node);
                cout << endl;
            }
        }
        sc_iterator3_free(nodes_it);
    }
    sc_iterator5_free(it);
}
//из названия все понятно))
sc_addr get_other_vertex_incidence_edge(sc_addr edge, sc_addr vertex)
{
    sc_addr v1, v2, empty;
    empty.seg = 0;
    empty.offset = 0;

    get_edge_vertexes(edge, v1, v2);
    if ((SC_ADDR_IS_EQUAL(vertex, v1)) || (SC_ADDR_IS_EQUAL(vertex, v2))) {
        if (SC_ADDR_IS_EQUAL(vertex, v1)) {
            return v2;
        } else {
            return v1;
        }
    }
    return empty;
}
//рисуем путь на экране
void print_route(sc_addr beg, sc_addr end)
{
    //cout<<'1'<<endl;
    printEl(context, end);


    sc_addr curr_vert, curr_ver = end;

    while (true) {
        sc_iterator5 *arcs = sc_iterator5_f_a_a_a_f_new(context,
                             curr_ver,
                             sc_type_arc_common,
                             0,
                             sc_type_arc_pos_const_perm,
                             father);
        if (SC_TRUE == sc_iterator5_next(arcs)) {
            curr_vert = sc_iterator5_value(arcs, 2);
            cout << "<-";
            printEl(context, curr_vert);
            //cout<<'2'<<endl;
            if (SC_ADDR_IS_EQUAL(curr_vert, beg)) break;
            curr_ver = curr_vert;
            sc_iterator5_free(arcs);
        } else break;
    }

}
//путь записываем в узел и если надо восстанвливаем дуги
sc_addr write_route(sc_addr beg, sc_addr end)
{
    //cout<<'1'<<endl;
    //print_graph();
    sc_addr set_future=sc_memory_node_new(context, sc_type_const);
    sc_addr curr_vert, curr_ver = end;
    while (true) {
        sc_iterator5 *arcs = sc_iterator5_f_a_a_a_f_new(context,
                             curr_ver,
                             sc_type_arc_common,
                             0,
                             sc_type_arc_pos_const_perm,
                             father);
        if (SC_TRUE == sc_iterator5_next(arcs)) {
            curr_vert = sc_iterator5_value(arcs, 2);
            sc_memory_arc_new(context, sc_type_arc_pos_const_perm, set_future,curr_vert);
            //cout<<'2'<<endl;
            if (SC_ADDR_IS_EQUAL(curr_vert, beg)) break;
            curr_ver = curr_vert;
            sc_iterator5_free(arcs);
        } else break;
    }
    if (helpme==SC_TRUE)
    {
        helpme=SC_FALSE;
        sc_iterator5 *vertexes = sc_iterator5_f_a_a_a_f_new(context,
                                 graph,
                                 sc_type_arc_pos_const_perm,
                                 0,
                                 sc_type_arc_pos_const_perm,
                                 rrel_arcs);
        sc_iterator5_next(vertexes);
        sc_addr set=sc_iterator5_value(vertexes,2);
        sc_addr t1=sc_memory_arc_new(context, sc_type_arc_common, beg,end);
        sc_addr t2=sc_memory_arc_new(context, sc_type_arc_common, end,beg);
        sc_memory_arc_new(context, sc_type_arc_pos_const_perm, set,t1);
        sc_memory_arc_new(context, sc_type_arc_pos_const_perm, set,t2);
        //print_graph();
    }
    return set_future;
}
//создание волны
sc_addr create_wave(sc_addr wave, sc_addr &not_checked_vertexes)
{
    sc_addr new_wave = sc_memory_node_new(context, sc_type_const);

    sc_iterator3 *it_vertex = sc_iterator3_f_a_a_new(context,
                              wave,
                              sc_type_arc_pos_const_perm,
                              0);

    while (SC_TRUE == sc_iterator3_next(it_vertex)) {
        sc_addr vertex = sc_iterator3_value(it_vertex, 2);

        sc_iterator5 *arcs = sc_iterator5_f_a_a_a_f_new(context,
                             graph,
                             sc_type_arc_pos_const_perm,
                             0,
                             sc_type_arc_pos_const_perm,
                             rrel_arcs);
        if (SC_TRUE == sc_iterator5_next(arcs)) {
            sc_addr set_arcs = sc_iterator5_value(arcs, 2);
            sc_iterator3 *it_arc = sc_iterator3_f_a_a_new(context,
                                   set_arcs,
                                   sc_type_arc_pos_const_perm,
                                   0);
            while (SC_TRUE == sc_iterator3_next(it_arc)) {
                sc_addr t_arc = sc_iterator3_value(it_arc, 2);
                sc_addr other_vertex = get_other_vertex_incidence_edge(t_arc, vertex);

                if (SC_ADDR_IS_EMPTY(other_vertex)) {
                    continue;
                }
                sc_iterator3 *find = sc_iterator3_f_a_f_new(context,
                                     not_checked_vertexes,
                                     sc_type_arc_pos_const_perm,
                                     other_vertex);

                if (SC_TRUE == sc_iterator3_next(find)) {
                    sc_addr edge = sc_iterator3_value(find, 1);
                    sc_memory_element_free(context, edge);
                    sc_memory_arc_new(context, sc_type_arc_pos_const_perm, new_wave, other_vertex);
                    /**/
                    sc_addr boof = sc_memory_arc_new(context, sc_type_arc_common, other_vertex, vertex);
                    sc_memory_arc_new(context, sc_type_arc_pos_const_perm, father, boof);
                    // sc_memory_element_free(boof);
                    /**/
                }
            }
        }
    }

    if (SC_TRUE == set_is_not_empty(new_wave)) {
        return new_wave;
    } else {
        sc_memory_element_free(context, new_wave);
        sc_addr new_wave;
        new_wave.seg = 0;
        new_wave.offset = 0;
        return new_wave;
    }
}
//есть элемент во множестве
sc_bool check_vertex(sc_addr Checked,sc_addr element)
{
    sc_bool answer=SC_FALSE;
    sc_int num,nym;
    num=nym=0;
    sc_iterator3 *yes=sc_iterator3_f_a_a_new(context,
                                             Checked,
                                             sc_type_arc_pos_const_perm,
                                             0);
    while (SC_TRUE == sc_iterator3_next(yes)) {
        sc_addr Test=sc_iterator3_value(yes,2);
        if (SC_ADDR_IS_NOT_EQUAL(Test, element))
            num++;
        nym++;
    }
    if (num!=nym)
        answer=SC_TRUE;
    return answer;
}
//проверяет есть ли дуга между вершинами
sc_bool Vernoli(sc_addr start,sc_addr end)
{
    sc_bool ans=SC_FALSE;
    sc_bool ans2=SC_FALSE;
    sc_bool ans1=SC_FALSE;
    sc_iterator3 *lol=sc_iterator3_f_a_f_new(context,
                                            start,
                                            sc_type_arc_common,
                                            end);
    if (SC_TRUE==sc_iterator3_next(lol))
    {
        ans1=SC_TRUE;
    }
    sc_iterator3 *lol1=sc_iterator3_f_a_f_new(context,
                                            end,
                                            sc_type_arc_common,
                                            start);
    if (SC_TRUE==sc_iterator3_next(lol1))
    {
        ans2=SC_TRUE;
    }
    if (ans1==SC_TRUE && ans2==SC_TRUE)
        ans=SC_TRUE;
    return ans;
}
//удаление дуг между вершинами
void Delete (sc_addr start,sc_addr end)
{
    sc_bool ans2=SC_FALSE;
    sc_bool ans1=SC_FALSE;
    sc_addr y2,y1;
    sc_iterator3 *lol=sc_iterator3_f_a_f_new(context,
                                            start,
                                            sc_type_arc_common,
                                            end);
    while (SC_TRUE==sc_iterator3_next(lol))
    {
        y1=sc_iterator3_value(lol,1);
        sc_memory_element_free(context,y1);
        ans1=SC_TRUE;
    }
    sc_iterator3 *lol1=sc_iterator3_f_a_f_new(context,
                                            end,
                                            sc_type_arc_common,
                                            start);
    while (SC_TRUE==sc_iterator3_next(lol1))
    {
        y2=sc_iterator3_value(lol1,1);
        sc_memory_element_free(context,y2);
        ans2=SC_TRUE;
    }
}
// волновой алгоритм поиска минимального пути, костыль с рядом стоящей вершиной работает++
// checked множество содержащее елементы которые нельзя внести в не проверенные
// shet счетчик от костыля
sc_addr find_min_path(sc_addr beg_vertex, sc_addr end_vertex,sc_addr Checked,sc_int shet)
{
    sc_addr empty;
    empty.offset = 0;
    empty.seg = 0;
    if (shet>0  && Vernoli(end_vertex,beg_vertex)==SC_TRUE)
    {
        Delete(beg_vertex,end_vertex);
        helpme=SC_TRUE;
        //print_graph();
    }
    sc_bool check = SC_FALSE;
    sc_addr not_checked_vertexes = sc_memory_node_new(context, sc_type_const);

    sc_iterator5 *vertexes = sc_iterator5_f_a_a_a_f_new(context,
                             graph,
                             sc_type_arc_pos_const_perm,
                             0,
                             sc_type_arc_pos_const_perm,
                             rrel_nodes);

    if (SC_TRUE == sc_iterator5_next(vertexes)) {
        sc_addr set_vertexes = sc_iterator5_value(vertexes, 2);
        sc_iterator3 *it_vertex = sc_iterator3_f_a_a_new(context,
                                  set_vertexes,
                                  sc_type_arc_pos_const_perm,
                                  0);
        while (SC_TRUE == sc_iterator3_next(it_vertex)) {
            sc_addr curr_vertex = sc_iterator3_value(it_vertex, 2);

            if (SC_FALSE==check_vertex(Checked,curr_vertex)) {
                if (SC_FALSE == find_vertex_in_set(curr_vertex, not_checked_vertexes))
                    sc_memory_arc_new(context, sc_type_arc_pos_const_perm, not_checked_vertexes, curr_vertex);
            }
        }
    }

    sc_addr new_wave = sc_memory_node_new(context, sc_type_const);
    sc_memory_arc_new(context, sc_type_arc_pos_const_perm, new_wave, beg_vertex);

    sc_addr wave_list = sc_memory_node_new(context, sc_type_const);
    sc_addr wave_list_tail = sc_memory_arc_new(context, sc_type_arc_pos_const_perm, wave_list, new_wave);

    do {
        new_wave = create_wave(new_wave, not_checked_vertexes);


        if (SC_ADDR_IS_EMPTY(new_wave)) {
            sc_memory_element_free(context, wave_list);
            sc_memory_element_free(context, new_wave);
            sc_memory_element_free(context, not_checked_vertexes);
            return empty;
        }

        wave_list_tail = sc_memory_arc_new(context, sc_type_arc_pos_const_perm, wave_list, new_wave);

        sc_iterator3 *find_end = sc_iterator3_f_a_a_new(context,
                                 new_wave,
                                 sc_type_arc_pos_const_perm,
                                 0);

        while (SC_TRUE == sc_iterator3_next(find_end)) {
            sc_addr tmp_vertex = sc_iterator3_value(find_end, 2);
            if (SC_ADDR_IS_EQUAL(tmp_vertex, end_vertex)) {
                check = SC_TRUE;
                continue;
            }
        }

    }

    while (check != SC_TRUE);

    sc_memory_element_free(context, not_checked_vertexes);

    empty = sc_memory_node_new(context, sc_type_const);
    return empty;
}
//ф-ия для объединения множеств
sc_addr Merge (sc_addr mer1, sc_addr mer2)
{
    sc_iterator3 *kek=sc_iterator3_f_a_a_new(context,
                                             mer2,
                                             sc_type_arc_pos_const_perm,
                                             0);
    while (SC_TRUE==sc_iterator3_next(kek))
    {
        sc_addr hope=sc_iterator3_value(kek,2);
        if (SC_FALSE==check_vertex(mer1,hope))
            sc_memory_arc_new(context, sc_type_arc_pos_const_perm, mer1,hope);
    }
    return mer1;
}
//функция для отладки, выводит на экран множество
void Print_set(sc_addr el)
{
    sc_iterator3 *kek=sc_iterator3_f_a_a_new(context,
                                             el,
                                             sc_type_arc_pos_const_perm,
                                             0);
    while (SC_TRUE==sc_iterator3_next(kek))
    {
        sc_addr hope=sc_iterator3_value(kek,2);
        printEl(context, hope);
        cout<<" ";
    }
    cout<<endl;
}
//функция запуска coutы для проверки, работает+++
void run_test(char number_test)
{
    sc_int answer,shetchik;
    father = sc_memory_node_new(context, sc_type_const);

    char gr[3] = "Gx";
    gr[1] = number_test;
    sc_helper_resolve_system_identifier(context, gr, &graph);
    sc_helper_resolve_system_identifier(context, "rrel_arcs", &rrel_arcs);
    sc_helper_resolve_system_identifier(context, "rrel_nodes", &rrel_nodes);
    cout << "Graph: ";
    print_graph();
    sc_iterator5 *vertexes = sc_iterator5_f_a_a_a_f_new(context,
                             graph,
                             sc_type_arc_pos_const_perm,
                             0,
                             sc_type_arc_pos_const_perm,
                             rrel_nodes);
    if (SC_TRUE == sc_iterator5_next(vertexes)) {
        sc_addr qwerty = sc_iterator5_value(vertexes, 2);
        sc_iterator3 *Bust1 = sc_iterator3_f_a_a_new(context,
                                  qwerty,
                                  sc_type_arc_pos_const_perm,
                                  0);
        shetchik=0;
            while (SC_TRUE == sc_iterator3_next(Bust1))
            {
                sc_iterator3 *Bust2 = sc_iterator3_f_a_a_new(context,
                                          qwerty,
                                          sc_type_arc_pos_const_perm,
                                          0);
                sc_addr x1=sc_iterator3_value(Bust1,2);
                while (SC_TRUE==sc_iterator3_next(Bust2))
                {
                    sc_addr x2=sc_iterator3_value(Bust2,2);
                    if (SC_ADDR_IS_EQUAL(x1,x2)==SC_FALSE)
                    {
                        sc_int shet=0;
                        sc_addr Checked= sc_memory_node_new(context,sc_type_const);
                        sc_memory_arc_new(context, sc_type_arc_pos_const_perm, Checked,x1);
                        //Print_set(Checked);
                        //printEl(context, x1);
                        //printEl(context, x2);
                        //cout<<Vernoli(x2,x1);
                        //cout<<Vernoli(x1,x2);
                        //cout<<endl;
                        sc_addr lebel = find_min_path(x1, x2,Checked,shet);
                        sc_addr elem=write_route(x1,x2);
                        while (SC_TRUE == sc_memory_is_element(context, lebel))
                        {
                             shet++;
                             Checked=Merge(Checked,elem);
                             //Print_set(Checked);
                             lebel=find_min_path(x1,x2,Checked,shet);
                             elem=write_route(x1,x2);
                        }
                        //cout<<shet<<endl;
                        if (shetchik==0)
                        {
                            shetchik++;
                            answer=shet;
                        }
                        else
                        {
                            if (shet<answer)
                                answer=shet;
                        }
                    }
                }
            }
        }
    cout<<"Число вершинной связности"<<" "<<answer;
//    cout << "Path";
//    if (SC_TRUE == sc_memory_is_element(context, lebel)) {
//        cout << ": " << endl;
//        print_route(beg, end);
//        sc_memory_element_free(context, lebel);
//    } else {
//        cout << " doesn't exist" << endl;
//    }

    cout << endl;
    sc_memory_element_free(context, father);
}

int main()
{
    sc_memory_params params;

    sc_memory_params_clear(&params);
    params.repo_path = "/home/kozel-stas/ostis/kb.bin";
    params.config_file = "/home/kozel-stas/ostis/config/sc-web.ini";
    params.ext_path = "/home/kozel-stas/ostis/sc-machine/bin/extensions";
    params.clear = SC_FALSE;

    sc_memory_initialize(&params);

    context = sc_memory_context_new(sc_access_lvl_make_max);


    //////////////////////////////////////////////////////////////////////////////////
    setlocale(LC_ALL,"rus");
    //run_test('0');
    //run_test('1');
    //run_test('2');
  //тесты работают только по одному исправить!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
    cout << "The end" << endl;

    sc_memory_context_free(context);

    sc_memory_shutdown(SC_TRUE);

    return 0;
}
